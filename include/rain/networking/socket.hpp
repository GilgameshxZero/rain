// Basic managed RAII Socket type, encapsulating a NativeSocket.
//
// All Socket implementations derive from here, directly or indirectly, or
// encapsulate a Socket. Interfaces should always be virtually inherited from,
// and may not track state.
#pragma once

#include "../algorithm/algorithm.hpp"
#include "../error/consume-throwable.hpp"
#include "../time/time.hpp"
#include "../time/timeout.hpp"
#include "exception.hpp"
#include "host.hpp"
#include "native-socket.hpp"
#include "resolve.hpp"
#include "specification.hpp"
#include "wsa.hpp"

#include <memory>

namespace Rain::Networking {
	// Basic managed RAII Socket type, encapsulating a (non-blocking)
	// NativeSocket. Sockets are not inherently thread-safe.
	//
	// The Interface does not track any internal state and should be virtually
	// inherited from for anything which expects a similar interface. The
	// non-Interface manages the kernel socket resource.
	class SocketInterface : virtual public SocketFamilyInterface,
													virtual public SocketTypeInterface,
													virtual public SocketProtocolInterface {
		public:
		// Import names for external subclass ease-of-use.
		using NativeSocket = NativeSocket;
		using Host = Host;

		// Sockets implementing this interface cannot be copied nor moved. In
		// addition, for resource management polymorphism, their destructor must be
		// virtual.
		SocketInterface() = default;
		virtual ~SocketInterface() {}
		SocketInterface(SocketInterface const &) = delete;
		SocketInterface &operator=(SocketInterface const &) = delete;
		SocketInterface(SocketInterface &&) = delete;
		SocketInterface &operator=(SocketInterface &&) = delete;

		protected:
		virtual NativeSocket nativeSocket() const noexcept = 0;

		// Set a new socket to be non-blocking
		void unblock() {
			// All Sockets are non-blocking. A change in this will also change derived
			// class interfaces.
#ifdef RAIN_PLATFORM_WINDOWS
			u_long ioctlOpt{1};
			validateSystemCall(ioctlsocket(
#else
			int ioctlOpt{1};
			validateSystemCall(ioctl(
#endif
				this->nativeSocket(), FIONBIO, &ioctlOpt));
		}

		// poll is enabled on all non-blocking sockets, and this encapsulates the
		// events which may be returned/passed into poll.
		enum class PollFlag : short {
			NONE = 0,

#ifdef RAIN_PLATFORM_WINDOWS
			READ_NORMAL = POLLRDNORM,
			WRITE_NORMAL = POLLWRNORM,
#else
			READ_NORMAL = POLLIN,
			WRITE_NORMAL = POLLOUT,
#endif

			// The following flags cannot be specified in events, but may be
			// returned in revents.
			POLL_ERROR = POLLERR,
			HANG_UP = POLLHUP,
			INVALID = POLLNVAL,
			PRIORITY = POLLPRI
		};
		friend PollFlag operator|(
			PollFlag const &left,
			PollFlag const &right) noexcept;
		friend PollFlag operator&(
			PollFlag const &left,
			PollFlag const &right) noexcept;

		// Additionally, implementing classes must provide a (protected) constructor
		// directly from a NativeSocket.

		// Code-sharing: SocketInterface defines a non-blocking Socket, and all
		// non-blocking Sockets are pollable.
		//
		// poll is inefficient for large numbers of Sockets as it must go through
		// all O(N) input Sockets each time any of them trigger an event.
		static std::vector<PollFlag> poll(
			std::vector<NativeSocket> const &nativeSockets,
			std::vector<PollFlag> const &events,
			Time::Timeout timeout = 15s) {
			std::vector<pollfd> fds;
			for (std::size_t index{0}; index < nativeSockets.size(); index++) {
				fds.push_back(
					{nativeSockets[index], static_cast<short>(events[index]), 0});
			}

#ifdef RAIN_PLATFORM_WINDOWS
			int ret = WSAPoll(
#else
			int ret = ::poll(
#endif
				fds.data(), static_cast<unsigned long>(fds.size()), timeout.asInt());

			if (ret == 0) {
				// Timeout.
				return std::vector<PollFlag>(fds.size(), PollFlag::NONE);
			} else if (ret == NATIVE_SOCKET_ERROR) {
				throw Networking::Exception(Networking::getSystemError());
			} else {
				// If any revents contains POLLNVAL, throw an error. Otherwise, return
				// the set of revents.
				std::vector<PollFlag> rEvents;
				for (pollfd const &fd : fds) {
					if (fd.revents & static_cast<short>(PollFlag::INVALID)) {
						throw Exception(Error::POLL_INVALID);
					}
					rEvents.push_back(static_cast<PollFlag>(fd.revents));
				}
				return rEvents;
			}
		}
		static std::vector<PollFlag> poll(
			std::vector<SocketInterface *> const &sockets,
			std::vector<PollFlag> const &events,
			Time::Timeout timeout = 15s) {
			std::vector<NativeSocket> nativeSockets;
			for (auto socket : sockets) {
				nativeSockets.push_back(socket->nativeSocket());
			}
			return SocketInterface::poll(nativeSockets, events, timeout);
		}

		// Single-Socket poll operates on the nativeSocket in the SocketInterface
		// implementer.
		//
		// Virtual to allow for Worker override to poll with interrupt Socket too.
		virtual PollFlag poll(PollFlag event, Time::Timeout timeout = 15s) {
			return SocketInterface::poll({this}, {event}, timeout)[0];
		}
	};

	// Bitwise operators for PollFlag.
	inline SocketInterface::PollFlag operator|(
		SocketInterface::PollFlag const &left,
		SocketInterface::PollFlag const &right) noexcept {
		return static_cast<SocketInterface::PollFlag>(
			static_cast<short>(left) | static_cast<short>(right));
	}
	inline SocketInterface::PollFlag operator&(
		SocketInterface::PollFlag const &left,
		SocketInterface::PollFlag const &right) noexcept {
		return static_cast<SocketInterface::PollFlag>(
			static_cast<short>(left) & static_cast<short>(right));
	}

	template <
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename> class...>
	class Socket;

	// Implements SocketInterface with concrete resource management for a
	// default non-blocking Socket.
	//
	// Open on construct, close on destruct. This is the only resource-managing
	// Socket class, and should always be instantiated after F/T/P are set, and
	// before socket options.
	template <
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface>
	class Socket<
		SocketFamilyInterface,
		SocketTypeInterface,
		SocketProtocolInterface> : virtual public SocketFamilyInterface,
															 virtual public SocketTypeInterface,
															 virtual public SocketProtocolInterface,
															 virtual public SocketInterface {
		private:
		NativeSocket _nativeSocket;

		public:
		Socket()
				: _nativeSocket(validateSystemCall(
						::socket(
							static_cast<int>(this->family()),
							static_cast<int>(this->type()),
							static_cast<int>(this->protocol())))) {
			this->unblock();
		}
		virtual ~Socket() {
			// Errors on Socket destruction are worth logging but not worth crashing
			// over.
			Rain::Error::consumeThrowable(
				RAIN_FUNCTIONAL_RESOLVE_OVERLOAD(validateSystemCall),
				RAIN_ERROR_LOCATION)(
#ifdef RAIN_PLATFORM_WINDOWS
				::closesocket(this->_nativeSocket));
#else
				::close(this->_nativeSocket));
#endif
		}

		protected:
		virtual NativeSocket nativeSocket() const noexcept final override {
			return this->_nativeSocket;
		}

		// Constructor variant takes an open NativeSocket (from accept).
		Socket(NativeSocket nativeSocket) : _nativeSocket(nativeSocket) {
			// Accepted sockets may need options reset on POSIX (inherited on
			// Windows).
			this->unblock();
		}

		// Friend so ServerSocket can make pairs.
		template <typename, typename>
		friend class ServerSocketSpec;

		// A resource-managing Socket (this class) provides the ability to swap
		// with another resource-managing Socket. Both are left in a valid state,
		// but this may cause unnecessary kernel calls if used incorrectly.
		void swap(Socket *other) noexcept {
			std::swap(this->_nativeSocket, other->_nativeSocket);
		}
	};

	// This is shorthand for defining a base resource-holding socket with F/T/P
	// and options, minimizing the number of wrapping templates at definition
	// site.
	template <
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename> class SocketOption,
		template <typename> class... SocketOptions>
	class Socket<
		SocketFamilyInterface,
		SocketTypeInterface,
		SocketProtocolInterface,
		SocketOption,
		SocketOptions...>
			: public SocketOption<Socket<
					SocketFamilyInterface,
					SocketTypeInterface,
					SocketProtocolInterface,
					SocketOptions...>> {
		using SocketOption<Socket<
			SocketFamilyInterface,
			SocketTypeInterface,
			SocketProtocolInterface,
			SocketOptions...>>::SocketOption;
	};

	// The direct subclass of Socket(Interface) is NamedSocket(Interface),
	// which is subclassed again by ConnectedSocket(Interface).
	//
	// A NamedSocket allows for getsockname and lookup of the peer hostname.
	class NamedSocketSpecInterface : virtual public SocketInterface {
		public:
		AddressInfo name() const {
			AddressInfo addressInfo;
			addressInfo.addressLen = sizeof(addressInfo.address);

			// This reinterpret_cast is never dereferenced here, and thus does not
			// break strict aliasing.
			validateSystemCall(getsockname(
				this->nativeSocket(),
				reinterpret_cast<sockaddr *>(&addressInfo.address),
				&addressInfo.addressLen));

			// RVO guaranteed, will be moved at worst.
			return addressInfo;
		}
		Host host() const { return getNumericHost(this->name()); }
	};

	// No-op.
	template <typename Socket>
	class NamedSocketSpec : public Socket,
													virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	// A ConnectedSocket(Interface) subclasses NamedSocket(Interface) and
	// additionally allows for send/recv (with the base assumption of a
	// non-blocking Socket).
	//
	// Since this is a non-blocking Socket,
	class ConnectedSocketSpecInterface : virtual public NamedSocketSpecInterface {
		public:
		// Throws if peer aborts. Returns 0 on timeout. Sends as many bytes as
		// possible before timeout.
		std::size_t send(
			char const *buffer,
			std::size_t bufferLen,
			Time::Timeout timeout = 15s) {
			std::size_t bytesSent{0};

			while (bytesSent < bufferLen) {
				// Poll until timeout or writeable so that send doesn't block.
				if (
					(this->poll(PollFlag::WRITE_NORMAL, timeout) &
					 PollFlag::WRITE_NORMAL) == PollFlag::NONE) {
					return bytesSent;
				}

				bytesSent += validateSystemCall(
					::send(
						this->nativeSocket(),
#ifdef RAIN_PLATFORM_WINDOWS
						buffer,
						static_cast<int>(bufferLen),
						0));
#else
						reinterpret_cast<const void *>(buffer),
						bufferLen,
						// IMPORTANT! sending to a disconnected client on POSIX may generate
						// SIGPIPE.
						MSG_NOSIGNAL));
#endif
			}
			return bytesSent;	 // Should equal bufferLen.
		}
		std::size_t send(std::string const &buffer, Time::Timeout timeout = 15s) {
			return this->send(&buffer[0], buffer.length(), timeout);
		}

		// Throws if peer aborts. Returns 0 on graceful close OR timeout. Check
		// for either case by checking if the timeout has passed.
		std::size_t
		recv(char *buffer, std::size_t bufferLen, Time::Timeout timeout = 15s) {
			// Poll until timeout or readable so that recv doesn't block.
			if (
				(this->poll(PollFlag::READ_NORMAL, timeout) & PollFlag::READ_NORMAL) ==
				PollFlag::NONE) {
				return 0;
			}

			return validateSystemCall(
				::recv(
					this->nativeSocket(),
#ifdef RAIN_PLATFORM_WINDOWS
					buffer,
					static_cast<int>(bufferLen),
#else
					reinterpret_cast<void *>(buffer),
					bufferLen,
#endif
					0));
		}
		std::size_t recv(std::string &buffer, Time::Timeout timeout = 15s) {
			buffer.resize(buffer.capacity());
			std::size_t result{this->recv(&buffer[0], buffer.length(), timeout)};
			buffer.resize(result);
			return result;
		}

		// Allow classic shutdown parameters, in addition to "graceful" shutdown,
		// which shuts down write; recvs remaining data, and then shuts down read.
		//
		// A timeout may be provided for the blocking GRACEFUL shutdown. It may be
		// checked by checking whether the timeout has passed.
		enum class ShutdownOpt { READ = 1, WRITE, BOTH, GRACEFUL };
		void shutdown(
			ShutdownOpt opt = ShutdownOpt::GRACEFUL,
			Time::Timeout timeout = 15s) {
			auto const shutdownRead = [this]() {
				validateSystemCall(
					::shutdown(
						this->nativeSocket(),
#ifdef RAIN_PLATFORM_WINDOWS
						SD_RECEIVE));
#else
						SHUT_RD));
#endif
			};
			auto const shutdownWrite = [this]() {
				validateSystemCall(
					::shutdown(
						this->nativeSocket(),
#ifdef RAIN_PLATFORM_WINDOWS
						SD_SEND));
#else
						SHUT_WR));
#endif
			};

			switch (opt) {
				case ShutdownOpt::READ:
					shutdownRead();
					break;
				case ShutdownOpt::WRITE:
					shutdownWrite();
					break;
				case ShutdownOpt::BOTH:
					validateSystemCall(
						::shutdown(
							this->nativeSocket(),
#ifdef RAIN_PLATFORM_WINDOWS
							SD_BOTH));
#else
							SHUT_RDWR));
#endif
					break;
				case ShutdownOpt::GRACEFUL:
				default:
					shutdownWrite();

					// At this point, it is plausible the peer has disconnected, so any
					// additional calls are expected to error.

					// Receive remaining, then shutdown read. Consume exceptions caused
					// by peer abort.
					Rain::Error::consumeThrowable([this, timeout]() {
						char buffer[1_zu << 10];
						while (this->recv(buffer, sizeof(buffer), timeout));
					})();
					break;
			}
		}

		// Similar to name() and host(), but this retrieves the name/host of the
		// peer.
		AddressInfo peerName() const {
			AddressInfo addressInfo;
			addressInfo.addressLen = sizeof(addressInfo.address);

			// This reinterpret_cast is never dereferenced here, and thus does not
			// break strict aliasing.
			validateSystemCall(getpeername(
				this->nativeSocket(),
				reinterpret_cast<sockaddr *>(&addressInfo.address),
				&addressInfo.addressLen));

			// RVO guaranteed, will be moved at worst.
			return addressInfo;
		}
		Host peerHost() const { return getNumericHost(this->peerName()); }
	};

	// No-op.
	template <typename Socket>
	class ConnectedSocketSpec : public Socket,
															virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;
	};

	// Further specializations of Socket(Interface) are Client, Worker, and
	// Server. Additional options may be set on the Socket in
	// socket-options.hpp.
}

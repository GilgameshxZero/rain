// A platform-agnostic RAII OOP socket implementation.
// Not socket-safe by default.
// Throws std::system_error instead of returning error code.

#pragma once

// For gai_strerror.
#ifdef UNICODE
#undef UNICODE
#endif

#include "../error-exception/exception.hpp"
#include "../platform.hpp"
#include "../time.hpp"
#include "host.hpp"
#include "native-socket.hpp"

#include <functional>

namespace Rain::Networking {
	// There are four types of Sockets, expressed via inheritance: Client, Server,
	// Slave, and general Sockets, which each expose a different set of functions.
	// When a class inherits two types of Sockets, it needs to explicitly expose
	// which functions are called from which base class.
	class Socket {
		// Error handling for common errors.
		public:
		enum class Error {
#ifdef RAIN_WINDOWS
			WSA_E_INTR = WSAEINTR,
			WSA_E_ACCES = WSAEACCES,
			WSA_E_NOT_SOCK = WSAENOTSOCK,
			WSA_E_NOT_CONN = WSAENOTCONN,
#else
			E_ACCESS = EACCES,
			E_PIPE = EPIPE,
			E_NOT_CONN = ENOTCONN,
			E_ADDR_NOT_AVAIL = EADDRNOTAVAIL,
			E_ADDR_IN_USE = EADDRINUSE,
#endif
			E_AI_AGAIN = EAI_AGAIN,
			E_AI_BADFLAGS = EAI_BADFLAGS,
			E_AI_FAIL = EAI_FAIL,
			E_AI_EAI_FAMILYAGAIN = EAI_FAMILY,
			E_AI_MEMORY = EAI_MEMORY,
			E_AI_NONAME = EAI_NONAME,
			E_AI_SERVICE = EAI_SERVICE,
			E_AI_SOCKTYPE = EAI_SOCKTYPE,
			AT_EXIT_REGISTER = 65536,
			RECV_CLOSE_GRACEFUL,
			SELECT_ON_INVALID
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept { return "Rain::Networking::Socket"; }
			std::string message(int ev) const {
				switch (static_cast<Error>(ev)) {
#ifdef RAIN_WINDOWS
					case Error::WSA_E_INTR:
						return "Interrupted function call.";
					case Error::WSA_E_ACCES:
						return "Permission denied.";
					case Error::WSA_E_NOT_SOCK:
						return "Socket operation on nonsocket.";
					case Error::WSA_E_NOT_CONN:
						return "Socket is not connected.";
#else
					case Error::E_ACCESS:
						return "Permission denied.";
					case Error::E_PIPE:
						return "Broken pipe.";
					case Error::E_NOT_CONN:
						return "Transport endpoint is not connected.";
					case Error::E_ADDR_NOT_AVAIL:
						return "A nonexistent interface was requested or the requested "
									 "address was not local.";
					case Error::E_ADDR_IN_USE:
						return "Address already in use";
#endif
					case Error::E_AI_AGAIN:
					case Error::E_AI_BADFLAGS:
					case Error::E_AI_FAIL:
					case Error::E_AI_EAI_FAMILYAGAIN:
					case Error::E_AI_MEMORY:
					case Error::E_AI_NONAME:
					case Error::E_AI_SERVICE:
					case Error::E_AI_SOCKTYPE:
						return gai_strerror(ev);
					case Error::AT_EXIT_REGISTER:
						return "Failed to register `WSACleanup` with `atexit`.";
					case Error::RECV_CLOSE_GRACEFUL:
						return "`recv` closed gracefully.";
					case Error::SELECT_ON_INVALID:
						return "Attempted to `select` on invalid (closed) socket.";
					default:
						return "Generic.";
				}
			}
		};
		inline static ErrorCategory errorCategory;

		private:
		static std::error_condition makeErrorCondition(Error e) {
			return std::error_condition(static_cast<int>(e), errorCategory);
		}
		static ErrorException::Exception makeException(Error e) {
			return ErrorException::Exception(makeErrorCondition(e));
		}

		// Family, type, protocol options.
		public:
		enum class Family { UNSPEC = AF_UNSPEC, IPV4 = AF_INET };
		enum class Type { STREAM = SOCK_STREAM, RAW = SOCK_RAW };
		enum class Protocol { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

		// Internal socket parameters.
		private:
		NativeSocket nativeSocket;
		Family family;
		Type type;
		Protocol protocol;

		// Getters.
		public:
		NativeSocket getNativeSocket() const noexcept { return this->nativeSocket; }
		Family getFamily() const noexcept { return this->family; }
		Type getType() const noexcept { return this->type; }
		Protocol getProtocol() const noexcept { return this->protocol; }

		// Socket is value/open if its nativeSocket is not invalid.
		bool isValid() const noexcept {
			return this->nativeSocket != NATIVE_SOCKET_INVALID;
		}

		private:
		// Platform-agnostic helper for throwing error code if some return-value is
		// invalid.
		template <typename ReturnValueType, typename InvalidValueType>
		ReturnValueType assertOrThrow(ReturnValueType returnValue,
			InvalidValueType invalidValue) const {
			if (returnValue == invalidValue) {
#ifdef RAIN_WINDOWS
				throw makeException(static_cast<Error>(WSAGetLastError()));
#else
				throw makeException(static_cast<Error>(errno));
#endif
			}
			return returnValue;
		}

		// Constructors and destructors.
		public:
		// Registers a new socket with the system.
		Socket(Family family = Family::IPV4,
			Type type = Type::STREAM,
			Protocol protocol = Protocol::TCP)
				: family(family), type(type), protocol(protocol) {
			// Call WSA initializers in Windows, and register to cleanup at program
			// exit.
#ifdef RAIN_WINDOWS
			static bool startupCalledBefore = false;
			if (!startupCalledBefore) {
				WSADATA wsaData;
				startupCalledBefore = true;
				int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (status != 0) {
					throw makeException(static_cast<Error>(status));
				}

				status = atexit([]() {
					if (WSACleanup() == NATIVE_SOCKET_ERROR) {
						throw makeException(static_cast<Error>(WSAGetLastError()));
					}
				});
				if (status != 0) {
					throw makeException(Error::AT_EXIT_REGISTER);
				}
			}
#endif

			// Create the socket, to be closed with the system on object destruction.
			this->nativeSocket =
				this->assertOrThrow(::socket(static_cast<int>(this->family),
															static_cast<int>(this->type),
															static_cast<int>(this->protocol)),
					NATIVE_SOCKET_INVALID);
		}
		~Socket() {
			// nativeSocket may be invalid after a move constructor.
			if (this->nativeSocket == NATIVE_SOCKET_INVALID) {
				return;
			}

			// Ignore any errors or exceptions since destructors are noexcept.
			try {
				close();
			} catch (...) {
			}
		}

		// Private constructor allows for accept factory to create new Sockets.
		private:
		Socket(NativeSocket nativeSocket,
			Family family = Family::IPV4,
			Type type = Type::STREAM,
			Protocol protocol = Protocol::TCP)
				: nativeSocket(nativeSocket),
					family(family),
					type(type),
					protocol(protocol) {}

		public:
		// RAII: Explicitly forbid the copy constructor and the assignment operator.
		Socket(Socket const &) = delete;
		Socket &operator=(Socket const &) = delete;

		// Move constructor is the preferred way of constructing with Socket
		// returned from accept.
		Socket(Socket &&o) noexcept
				: nativeSocket(std::exchange(o.nativeSocket, NATIVE_SOCKET_INVALID)),
					family(o.family),
					type(o.type),
					protocol(o.protocol) {}

		// Connect and bind.
		private:
		// Shared code for connect and bind.
		void connectOrBind(Host const &host,
			std::function<void(addrinfo *)> const &setHints,
			std::function<int(NativeSocket, sockaddr *, int)> const &action) const {
			addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = static_cast<int>(this->family);
			hints.ai_socktype = static_cast<int>(this->type);
			hints.ai_protocol = static_cast<int>(this->protocol);
			setHints(&hints);
			int status = getaddrinfo(
				host.node.getCStr(), host.service.getCStr(), &hints, &result);
			if (status != 0) {
				throw makeException(static_cast<Error>(status));
			}

			// Try all the addresses we found.
			curAddr = result;
			while (curAddr != NULL) {
				if ((status = action(this->nativeSocket,
							 curAddr->ai_addr,
							 static_cast<int>(curAddr->ai_addrlen))) == 0) {
					break;
				}
				curAddr = curAddr->ai_next;
			}
			freeaddrinfo(result);
			this->assertOrThrow(status, NATIVE_SOCKET_ERROR);
		}

		public:
		void connect(Host const &host) const {
			this->connectOrBind(
				host, [](addrinfo *) {}, ::connect);
		}
		void bind(Host const &host) const {
			this->connectOrBind(
				host, [](addrinfo *hints) { hints->ai_flags = AI_PASSIVE; }, ::bind);
		}

		// For a connected socket, can get the service on which it is connected.
		Host::Service getService() const {
			struct sockaddr_in sin;
			socklen_t addrlen = sizeof(sin);
			this->assertOrThrow(getsockname(this->nativeSocket,
														reinterpret_cast<struct sockaddr *>(&sin),
														&addrlen),
				NATIVE_SOCKET_ERROR);
			return Host::Service(static_cast<std::size_t>(ntohs(sin.sin_port)));
		}

		// Listen.
		public:
		void listen(int backlog = SOMAXCONN) const {
			this->assertOrThrow(
				::listen(this->nativeSocket, backlog), NATIVE_SOCKET_ERROR);
		}

		private:
		// Convenience function for optionally-blocking functions. Returns true if
		// terminated on timeout. By default, blocks without timeout.
		// Throws an exception if nativeSocket is invalid (socket has been closed).
		bool blockForSelect(bool read,
			std::chrono::milliseconds const &timeoutMs) const {
			if (this->nativeSocket == NATIVE_SOCKET_INVALID) {
				throw makeException(Error::SELECT_ON_INVALID);
			}
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(this->nativeSocket, &fds);
			timeval tv;
			tv.tv_sec = static_cast<long>(timeoutMs.count()) / 1000;
			tv.tv_usec = (timeoutMs.count() % 1000) * 1000;

			// Timeout if `select` returns 0, and return true.
			return (
				this->assertOrThrow(select(static_cast<int>(this->nativeSocket) + 1,
															read ? &fds : NULL,
															read ? NULL : &fds,
															&fds,
															timeoutMs.count() == 0 ? NULL : &tv),
					NATIVE_SOCKET_ERROR) == 0);
		}

		// Accept uses select to block for new connections.
		// Timing out will return an invalid Socket.
		public:
		Socket accept(sockaddr *addr = NULL,
			socklen_t *addrLen = NULL,
			std::chrono::milliseconds const &timeoutMs =
				std::chrono::milliseconds::zero()) const {
			if (this->blockForSelect(true, timeoutMs)) {
				return Socket(NATIVE_SOCKET_INVALID, family, type, protocol);
			}
			return Socket(
				this->assertOrThrow(
					::accept(this->nativeSocket, addr, addrLen), NATIVE_SOCKET_INVALID),
				family,
				type,
				protocol);
		}

		// Custom types for send and recv flags.
		public:
		enum class SendFlag {
			NONE = 0,
#ifdef RAIN_WINDOWS
			NO_SIGNAL = 0,
#else
			NO_SIGNAL = MSG_NOSIGNAL,
#endif
			NO_ROUTE = MSG_DONTROUTE,
			OUT_OF_BAND = MSG_OOB
		};
		enum class RecvFlag { NONE = 0 };

		// Send and recv.
		public:
		// Block until send all bytes. Uses select to make sure we can write before
		// we try. Returns number of bytes sent (may be different from intended if
		// using timeout).
		std::size_t send(char const *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NONE,
			std::chrono::milliseconds const &timeoutMs =
				std::chrono::milliseconds::zero()) const {
			if (len == 0) {
				len = std::strlen(msg);
			}

			std::size_t bytesSent = 0;
			std::chrono::steady_clock::time_point timeoutTime =
				std::chrono::steady_clock::now() + timeoutMs;
			while (bytesSent != len) {
				if (this->blockForSelect(false,
							timeoutMs.count() == 0
								? std::chrono::milliseconds::zero()
								: std::chrono::duration_cast<std::chrono::milliseconds>(
										timeoutTime - std::chrono::steady_clock::now()))) {
					break;
				}
				bytesSent += static_cast<std::size_t>(
					this->assertOrThrow(::send(this->nativeSocket,
#ifdef RAIN_WINDOWS
																msg + bytesSent,
																static_cast<int>(len - bytesSent),
#else
																reinterpret_cast<const void *>(msg + bytesSent),
																len - bytesSent,
#endif
																static_cast<int>(flags)),
						NATIVE_SOCKET_ERROR));
			}
			return bytesSent;
		}
		std::size_t send(std::string const &s,
			SendFlag flags = SendFlag::NONE,
			std::chrono::milliseconds const &timeoutMs =
				std::chrono::milliseconds::zero()) const {
			return this->send(s.c_str(), s.length(), flags, timeoutMs);
		}

		// Block until recv some bytes, or exception if something went wrong.
		// Uses select to block to correctly break block if socket closed.
		// Returns positive number of bytes received, or 0 if timed out.
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::chrono::milliseconds const &timeoutMs =
				std::chrono::milliseconds::zero()) const {
			if (this->blockForSelect(true, timeoutMs)) {
				return 0;
			}
			int status = this->assertOrThrow(::recv(this->nativeSocket,
#ifdef RAIN_WINDOWS
																				 buf,
																				 static_cast<int>(len),
#else
																				 reinterpret_cast<void *>(buf),
																				 len,
#endif
																				 static_cast<int>(flags)),
				NATIVE_SOCKET_ERROR);
			if (status == 0) {
				throw makeException(Error::RECV_CLOSE_GRACEFUL);
			}
			return status;
		}

		// Manually shutdown read and write or close listening sockets.
		public:
		void shutdown() const {
			this->assertOrThrow(
#ifdef RAIN_WINDOWS
				::shutdown(this->nativeSocket, SD_BOTH),
#else
				::shutdown(this->nativeSocket, SHUT_RDWR),
#endif
				NATIVE_SOCKET_ERROR);
		}
		void close() {
			this->assertOrThrow(
#ifdef RAIN_WINDOWS
				closesocket(this->nativeSocket),
#else
				::close(this->nativeSocket),
#endif
				NATIVE_SOCKET_ERROR);
			this->nativeSocket = NATIVE_SOCKET_INVALID;
		}
	};
}

// Error handling, continued.
namespace std {
	template <>
	struct is_error_condition_enum<Rain::Networking::Socket::Error>
			: public true_type {};
}

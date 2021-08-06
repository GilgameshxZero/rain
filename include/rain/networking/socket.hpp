// RAII thread-safe socket.
#pragma once

#include "../error.hpp"
#include "../literal.hpp"
#include "../multithreading/unlock-guard.hpp"
#include "../platform.hpp"
#include "../time.hpp"
#include "exception.hpp"
#include "resolve.hpp"

#include <functional>
#include <iostream>
#include <mutex>

namespace Rain::Networking {
	// Flags for Socket-internal functions. Declared non-nested for operator
	// overloading. Flags for poll.
	enum class PollEvent : short {
		NONE = 0,

#ifdef RAIN_PLATFORM_WINDOWS
		READ_NORMAL = POLLRDNORM,
		WRITE_NORMAL = POLLWRNORM,
#else
		READ_NORMAL = POLLIN,
		WRITE_NORMAL = POLLOUT,
#endif

		// The following flags cannot be specified in events, but may be returned
		// in revents.
		POLL_ERROR = POLLERR,
		HANG_UP = POLLHUP,
		INVALID = POLLNVAL,
		PRIORITY = POLLPRI
	};

	// Flags for send. Add more if necessary.
	enum class SendFlag { NONE = 0 };

	// Flags for recv. Add more if necessary.
	enum class RecvFlag { NONE = 0 };

	// Custom operators for flag-like enum classes.
	inline PollEvent operator|(
		PollEvent const &left,
		PollEvent const &right) noexcept {
		return static_cast<PollEvent>(
			static_cast<short>(left) | static_cast<short>(right));
	}
	inline SendFlag operator|(
		SendFlag const &left,
		SendFlag const &right) noexcept {
		return static_cast<SendFlag>(
			static_cast<int>(left) | static_cast<int>(right));
	}
	inline RecvFlag operator|(
		RecvFlag const &left,
		RecvFlag const &right) noexcept {
		return static_cast<RecvFlag>(
			static_cast<int>(left) | static_cast<int>(right));
	}

	// RAII thread-safe socket.
	//
	// Sockets are specialized into 3 types: Clients, Servers, and Workers.
	// Specializations take a Socket type as template. Each protocol layer
	// implements their own protocol Socket deriving Socket, and protocol
	// specializations derive base specializations, but take their own protocol
	// Socket as template parameter. Protocol specializations must allow further
	// template arguments if further protocols are to built atop it. This lets us
	// avoid messy virtual inheritance. No specialization (marked Interface) is
	// ever to be instantiated; while they are not abstract, they are abstract in
	// the sense that to instantiate them, one must provide template arguments.
	//
	// Socket specializations/interface specializations appears to break LSP by
	// design. Socket upholds few contracts: most notably, if a function
	// completes, it suceeds, and it must throw on failure. Interface
	// specializations of Socket uphold this contract; however, by design, only
	// part of the Socket functionality is exposed to the caller as a
	// *recommendation* for generating fewer errors.
	//
	// By default, sockets are IPv6 dual-stack and non-blocking, and methods on
	// the socket make this assumption. While poll is available on Socket, it is
	// preferred to use other provided Socket functions with the timeout.
	//
	// Sockets may be interrupted from blocking polls by the use of the
	// interruptPair. An interrupt will timeout all future calls to poll.
	//
	// Sockets do NOT satisfy the NTBA (no-throw/block abort) contract but does
	// satisfy the lesser NTA contract. A Socket which is aborted (via its
	// destructor or abort) may block for the internal mutex and Socket operations
	// running on other threads to finish. A socket whose locking operations are
	// only called from a single thread, and interrupted before abort, is NBTA.
	//
	// List of Socket functions:
	//
	// using Interface::getNativeSocket;
	// using Interface::getSpecification;
	// using Interface::isValid;
	// using Interface::isInterruptable;
	// using Interface::getInterruptPair;
	// using Interface::setInterruptPair;
	// using Interface::setNewInterruptPair;
	// using Interface::getTargetAddress;
	// using Interface::getTargetHost;
	// using Interface::abort;
	// using Interface::shutdown;
	// using Interface::close;
	// using Interface::connect;
	// using Interface::bind;
	// using Interface::listen;
	// using Interface::accept;
	// using Interface::send;
	// using Interface::sendOnce;
	// using Interface::recv;
	// using Interface::interrupt;
	class Socket {
		public:
		enum class Error {
			// Carried over from Networking::Error.
			IN_PROGRESS = static_cast<int>(Networking::Error::IN_PROGRESS),
			WOULD_BLOCK = static_cast<int>(Networking::Error::WOULD_BLOCK),
			NOT_CONNECTED = static_cast<int>(Networking::Error::NOT_CONNECTED),

			NONE = 0,
			POLL_INVALID = (1_zu << 16),
			INTERRUPT_ON_UNINTERRUPTABLE
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept { return "Rain::Networking::Socket"; }
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::NONE:
						return "None.";
					case Error::POLL_INVALID:
						return "An invalid socket was passed to `poll`.";
					case Error::INTERRUPT_ON_UNINTERRUPTABLE:
						return "Attempted to interrupt an uninterruptable Socket.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		private:
		// Handle/fd of the underlying socket, specific to the platform.
		NativeSocket nativeSocket;

		// Socket specifications.
		Specification::Specification specification;

		private:
		// stateMtx is locked on all non-blocking operations which require
		// consistency. operationMtx is locked on operations which modify the
		// underlying socket. poll unlocks stateMtx while blocking.
		//
		// Public functions lock these. Private and protected functions assume they
		// are already locked.
		mutable std::mutex stateMtx, operationMtx;

		private:
		// To interrupt blocking calls (all implemented via poll), poll blocks on an
		// additional Socket. That Socket is part of a pair which can be written
		// to/read from to break poll.
		//
		// If the pair is unset, the Socket cannot be interrupted.
		std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>> interruptPair;

		// Tracks whether read/write has been shutdown.
		bool shutdownRead = false, shutdownWrite = false;

		public:
		// Getters for internal properties.

		// Get the native socket representation. It is preferred to operate on
		// sockets via the Socket interface rather than with the native socket.
		NativeSocket getNativeSocket() const noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->nativeSocket;
		}

		Specification::Specification getSpecification() const noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->specification;
		}

		private:
		bool _isValid() const noexcept {
			return this->nativeSocket != NATIVE_SOCKET_INVALID;
		}

		public:
		// A socket becomes invalid if it has been destructed or moved.
		bool isValid() const noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_isValid();
		}

		private:
		bool _isInterruptable() const noexcept {
			return static_cast<bool>(this->interruptPair.first);
		}

		public:
		// A Socket is interruptable iff its interruptPair is non-empty.
		// Uninterruptable sockets are most commonly used as the interruptPair
		// themselves.
		bool isInterruptable() const noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_isInterruptable();
		}
		std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>>
		getInterruptPair() const noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->interruptPair;
		}
		void setInterruptPair(
			std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>>
				interruptPair) noexcept {
			std::lock_guard stateLckGuard(this->stateMtx);
			this->interruptPair = interruptPair;
		}
		void setNewInterruptPair() {
			std::lock_guard stateLckGuard(this->stateMtx);
			this->interruptPair = Socket::createSocketPair();
		}

		private:
		// Utility function for a common function pattern. Throws a system-specific
		// error if some function return value is equal to an invalid value.
		template <typename ReturnValue, typename InvalidValue>
		ReturnValue validateOrThrow(
			ReturnValue const &returnValue,
			InvalidValue const &invalidValue) const {
			if (returnValue == invalidValue) {
				throw Networking::Exception(Networking::getSystemError());
			}

			return returnValue;
		}

		private:
		Resolve::AddressInfo _getTargetAddress() const {
			Resolve::AddressInfo addressInfo;
			socklen_t addressLen = sizeof(addressInfo.address);

			// This reinterpret_cast is never dereferenced here, and thus does not
			// break strict aliasing.
			this->validateOrThrow(
				getsockname(
					this->nativeSocket,
					reinterpret_cast<sockaddr *>(&addressInfo.address),
					&addressLen),
				NATIVE_SOCKET_ERROR);

			addressInfo.addressLen = static_cast<std::size_t>(addressLen);
			// Probably won't trigger (N)RVO, so we'll just invoke copy.
			return addressInfo;
		}

		public:
		// For a bound or connected socket, retrieves the host to which it is bound
		// or connected.
		Resolve::AddressInfo getTargetAddress() const {
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_getTargetAddress();
		}
		Host getTargetHost() const {
			// With the address and address length, we can now resolve it to a Host.
			std::lock_guard stateLckGuard(this->stateMtx);
			Resolve::AddressInfo addressInfo(this->_getTargetAddress());

			// RVO guaranteed.
			return Resolve::getNumericHost(
				addressInfo.address, addressInfo.addressLen);
		}

		// Standard constructor, which registers a new system socket.
		//
		// By default, Sockets are interruptable, unless the flag in the constructor
		// is set to false.
		Socket(
			Specification::Specification const &specification =
				{Specification::ProtocolFamily::INET6,
				 Specification::SocketType::STREAM,
				 Specification::SocketProtocol::TCP},
			bool interruptable = true)
				: nativeSocket(NATIVE_SOCKET_INVALID), specification(specification) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);

			Wsa::prepare();

			this->nativeSocket = this->validateOrThrow(
				::socket(
					static_cast<int>(specification.getProtocolFamily()),
					static_cast<int>(specification.getSocketType()),
					static_cast<int>(specification.getSocketProtocol())),
				NATIVE_SOCKET_INVALID);

			// Setup default socket options.
			this->setNonBlocking();
			this->setDualStack();
			this->setNoLinger();

			if (interruptable) {
				this->interruptPair = Socket::createSocketPair();
			}
		}

		// Creates a pair of connected Sockets to be used for interrupts on polls.
		//
		// Assumes that a socket pair will not be broken spuriously. Should not
		// block since local connection.
		static std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>>
		createSocketPair() {
			// Create a temporary Socket, bind and listen on it, then connect and
			// accept the pair.
			// Socket pairs are uninterruptable by default.
			Socket server(
				{Specification::ProtocolFamily::INET6,
				 Specification::SocketType::STREAM,
				 Specification::SocketProtocol::TCP},
				false);
			server.bind({"127.0.0.1:0"}, {});
			server.listen();

			// The first Socket is the accepted one, the second Socket is client.
			std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>> socketPair;
			std::thread acceptThread([&server, &socketPair]() {
				socketPair.first.reset(
					new Socket(std::move(server.accept(Time::Timeout<>()).first)));
			});

			socketPair.second.reset(new Socket(
				{Specification::ProtocolFamily::INET6,
				 Specification::SocketType::STREAM,
				 Specification::SocketProtocol::TCP},
				false));
			socketPair.second->connect(
				{"127.0.0.1", server.getTargetHost().service}, {});

			acceptThread.join();
			return socketPair;
		}

		private:
		// Sets default socket options for a new native socket. Used during
		// constructor, as well as on the socket after accept. This function need
		// only be called once per native socket. The default options are
		// non-blocking, dual stack (if IPv6), and no linger (abort) on close.
		void setNonBlocking() {
			// Sockets are non-blocking by default.
#ifdef RAIN_PLATFORM_WINDOWS
			u_long ioctlOpt = 1;
			this->validateOrThrow(
				ioctlsocket(
#else
			int ioctlOpt = 1;
			this->validateOrThrow(
				ioctl(
#endif
					this->nativeSocket, FIONBIO, &ioctlOpt),
				NATIVE_SOCKET_ERROR);
		}
		void setDualStack() {
			// If socket is IPv6, it is dual-stack by default.
			if (
				this->specification.getProtocolFamily() ==
				Specification::ProtocolFamily::INET6) {
#ifdef RAIN_PLATFORM_WINDOWS
				DWORD sockOpt = 0;
#else
				int sockOpt = 0;
#endif
				this->validateOrThrow(
					setsockopt(
						this->nativeSocket,
						IPPROTO_IPV6,
						IPV6_V6ONLY,
#ifdef RAIN_PLATFORM_WINDOWS
						reinterpret_cast<char const *>(&sockOpt),
#else
						reinterpret_cast<void const *>(&sockOpt),
#endif
						sizeof(sockOpt)),
					NATIVE_SOCKET_ERROR);
			}
		}
		void setNoLinger() {
			// By default, don't linger. Thus causes any close on the socket to abort
			// the connection immediately. If lingering is required, trigger it
			// manually via shutdown/send/recv.
			linger lingerOpt{1, 0};
			this->validateOrThrow(
				setsockopt(
					this->nativeSocket,
					SOL_SOCKET,
					SO_LINGER,
#ifdef RAIN_PLATFORM_WINDOWS
					reinterpret_cast<char const *>(&lingerOpt),
#else
					reinterpret_cast<void const *>(&lingerOpt),
#endif
					sizeof(lingerOpt)),
				NATIVE_SOCKET_ERROR);
		}

		void _abort() {
			if (!this->_isValid()) {
				return;
			}

			this->validateOrThrow(
#ifdef RAIN_PLATFORM_WINDOWS
				::closesocket(this->nativeSocket),
#else
				::close(this->nativeSocket),
#endif
				NATIVE_SOCKET_ERROR);
			this->nativeSocket = NATIVE_SOCKET_INVALID;
		}

		public:
		// Abort the connection immediately, discarding any data not yet sent from
		// buffer. To ensure data is sent, recommended to shutdown socket send, then
		// wait for graceful shutdown from recv with a timeout before abort.
		//
		// Calling abort multiple times will not throw, if the first call did not
		// throw.
		void abort() {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			this->_abort();
		}

		private:
		void _shutdown(bool write = true, bool read = false) {
			// Allow shutdown multiple times on the same flag.
			write &= !this->shutdownWrite;
			read &= !this->shutdownRead;
			this->shutdownRead |= read;
			this->shutdownWrite |= write;

			int how;
			if (write) {
				if (read) {
#ifdef RAIN_PLATFORM_WINDOWS
					how = SD_BOTH;
#else
					how = SHUT_RDWR;
#endif
				} else {
#ifdef RAIN_PLATFORM_WINDOWS
					how = SD_SEND;
#else
					how = SHUT_WR;
#endif
				}
			} else {
				if (read) {
#ifdef RAIN_PLATFORM_WINDOWS
					how = SD_RECEIVE;
#else
					how = SHUT_RD;
#endif
				} else {
					return;
				}
			}

			// If error is not connected, just ignore it. The other end might cause a
			// disconnect on our end by aborting on their end.
			try {
				this->validateOrThrow(
					::shutdown(this->nativeSocket, how), NATIVE_SOCKET_ERROR);
			} catch (Networking::Exception const &exception) {
				if (exception.getError() == Networking::Error::NOT_CONNECTED) {
					this->shutdownRead = this->shutdownWrite = true;
				} else {
					throw exception;
				}
			}
		}

		public:
		// Shutdown read/write. By default only shuts down write. Shutting down
		// write sends a FIN to peer. Does not invalidate socket until abort is
		// called. Does nothing if both read/write are false.
		void shutdown(bool write = true, bool read = false) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			this->_shutdown(write, read);
		}

		private:
		// recv until graceful close or timeout. Used during graceful close routine.
		// Returns true if timeout, false otherwise.
		template <typename Clock = std::chrono::steady_clock>
		bool recvRemaining(Time::Timeout<Clock> const &timeout = 60s) {
			std::size_t recvStatus = SIZE_MAX;

			// Consume exceptions silently.
			Rain::Error::consumeThrowable([this, &timeout, &recvStatus]() {
				char buffer[1_zu << 10];
				do {
					recvStatus = this->_recv(buffer, sizeof(buffer), timeout);
				} while (recvStatus != 0 && recvStatus != SIZE_MAX);
			})();

			if (recvStatus == SIZE_MAX) {
				return true;
			} else {
				this->shutdownRead = true;
				return false;
			}
		}

		template <typename Clock = std::chrono::steady_clock>
		bool _close(Time::Timeout<Clock> const &timeout = 60s) {
			if (!this->_isValid()) {
				return false;
			}

			this->_shutdown();
			bool timedOut = this->recvRemaining(timeout);
			this->_abort();
			return timedOut;
		}

		public:
		// Initiate a graceful close sequence. Send a FIN by shutting down socket
		// send, then recv until error or graceful shutdown, then close the socket.
		// Errors during recv are somewhat expected, so consume them and close
		// immediately if they arise. Use gracefulClose to ensure that any remaining
		// data in the socket is received by a well-behaving peer before close.
		// After gracefulClose, the socket is guaranteed to be invalid. A negative
		// timeout blocks indefinitely on the internal recv and is not recommended.
		//
		// Returns true on timeout, false otherwise.
		//
		// Calling close multiple times will not throw, if the first call did not
		// throw. If Socket was previously interrupted, close does not guarantee
		// graceful close.
		template <typename Clock = std::chrono::steady_clock>
		bool close(Time::Timeout<Clock> const &timeout = 60s) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_close(timeout);
		}

		// Abort. Polymorphic destructors are virtual to allow for deletion of
		// Derived class via Base class pointer.
		virtual ~Socket() {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			// Ignore any errors from close.
			Rain::Error::consumeThrowable(
				[this]() { this->_abort(); }, RAIN_ERROR_LOCATION)();
		}

		public:
		// RAII: Forbid copy construct and assignment.
		Socket(Socket const &) = delete;
		Socket &operator=(Socket const &) = delete;

		// Move construct used mostly from accept.
		Socket(Socket &&other)
				: nativeSocket(
						std::exchange(other.nativeSocket, NATIVE_SOCKET_INVALID)),
					specification(other.specification),
					interruptPair(other.interruptPair),
					shutdownRead(other.shutdownRead),
					shutdownWrite(other.shutdownWrite) {}

		private:
		// Block with `poll`, waiting on events on the underlying socket.
		// Utility enums are provided for the flags.
		// Return zero on timeout. Return `revents` on event. Throw exception
		// otherwise (always on INVALID). Prefer `poll` over `select` for modernity
		// reasons. A static multi-Socket variant of poll is provided, from which
		// the Socket-specific implementation derives. Multi-Socket poll is useful
		// for providing an additional Socket on which to interrupt a poll call,
		// specifically for Server::accept.
		template <typename Clock = std::chrono::steady_clock>
		static std::vector<PollEvent> poll(
			std::vector<PollEvent> const &events,
			std::vector<Socket const *> const &sockets,
			Time::Timeout<Clock> const &timeout = 60s) {
			// Unlock stateMtx for the duration of blocking poll.
			std::vector<std::unique_ptr<Multithreading::UnlockGuard<std::mutex>>>
				unlockGuards;

			std::vector<pollfd> fds;
			for (std::size_t index = 0; index < sockets.size(); index++) {
				fds.push_back(
					{sockets[index]->nativeSocket, static_cast<short>(events[index]), 0});
				unlockGuards.push_back(
					std::make_unique<Multithreading::UnlockGuard<std::mutex>>(
						sockets[index]->stateMtx));
			}

#ifdef RAIN_PLATFORM_WINDOWS
			int ret = WSAPoll(
#else
			int ret = ::poll(
#endif
				fds.data(),
				static_cast<unsigned long>(fds.size()),
				timeout.asClassicInt());

			// Lock stateMtx again.
			unlockGuards.clear();

			if (ret == 0) {
				// Timeout.
				return std::vector<PollEvent>(fds.size(), PollEvent::NONE);
			} else if (ret == NATIVE_SOCKET_ERROR) {
				throw Networking::Exception(Networking::getSystemError());
			} else {
				// If any revents contains POLLNVAL, throw an error. Otherwise, return
				// the set of revents.
				std::vector<PollEvent> revents;
				for (pollfd const &fd : fds) {
					if (fd.revents & static_cast<short>(PollEvent::INVALID)) {
						throw Socket::Exception(Socket::Error::POLL_INVALID);
					}
					revents.push_back(static_cast<PollEvent>(fd.revents));
				}
				return revents;
			}
		}

		// Single-Socket poll always calls with interrupt socket for
		// interruptability.
		template <typename Clock = std::chrono::steady_clock>
		PollEvent poll(
			PollEvent events = PollEvent::READ_NORMAL | PollEvent::WRITE_NORMAL,
			Time::Timeout<Clock> const &timeout = 60s) const {
			// Delegate to multi-Socket poll. Multi-socket poll will never mess with
			// any pointers, so unwrapping is fine here.
			if (this->_isInterruptable()) {
				// Poll requires that stateMtx be locked for all Sockets.
				std::lock_guard pairFirstStateLckGuard(
					this->interruptPair.first->stateMtx);
				std::lock_guard pairSecondStateLckGuard(
					this->interruptPair.second->stateMtx);
				return Socket::poll(
					{events, PollEvent::NONE, PollEvent::READ_NORMAL},
					{this,
					 this->interruptPair.first.get(),
					 this->interruptPair.second.get()},
					timeout)[0];
			} else {
				return Socket::poll({events}, {this}, timeout)[0];
			}
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		bool _connect(
			Resolve::AddressInfo const &addressInfo,
			Time::Timeout<Clock> const &timeout = 60s) {
			Error connectError = Error::NONE;
			if (
				::connect(
					this->nativeSocket,
					reinterpret_cast<sockaddr const *>(&addressInfo.address),
					static_cast<socklen_t>(addressInfo.addressLen)) ==
				NATIVE_SOCKET_ERROR) {
				connectError = static_cast<Error>(Networking::getSystemError());
			}

			switch (connectError) {
				case Error::NONE:
					// Though unlikely, non-blocking socket can connect immediately and
					// this is fine.
					return false;
				case static_cast<Error>(Networking::Error::IN_PROGRESS):
				case static_cast<Error>(Networking::Error::WOULD_BLOCK):
					// Both of these flags arise for non-blocking sockets, and necessitate
					// waiting for connect via poll.
					break;
				default:
					// Unexpected error.
					throw Networking::Exception(
						static_cast<Networking::Error>(connectError));
			}

			// connect has returned IN_PROGRESS or WOULD_BLOCK for this non-blocking
			// socket, and we should wait with poll. Ignore any potential error flags;
			// all we care about is whether connect succeeded.
			PollEvent rEvents = this->poll(PollEvent::WRITE_NORMAL, timeout);

			// Return false iff rEvents is WRITE_NORMAL. Otherwise, it may have timed
			// out (NONE) or errored out.
			return !(rEvents == PollEvent::WRITE_NORMAL);
		}

		public:
		// connect to an AddressInfo directly. Return false on success and true on
		// error or timeout.
		template <typename Clock = std::chrono::steady_clock>
		bool connect(
			Resolve::AddressInfo const &addressInfo,
			Time::Timeout<Clock> const &timeout = 60s) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_connect(addressInfo, timeout);
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		bool _connect(
			std::vector<Resolve::AddressInfo> const &addressInfos,
			bool parallel = true,
			Time::Timeout<Clock> const &timeout = 60s) {
			if (addressInfos.empty()) {
				return true;
			}

			// If parallel is set, spawns threads/sockets to try all of them in
			// parallel. Otherwise, tries each of them in series.
			if (parallel) {
				// Parallel connect consumes all exceptions.
				// Triggered when any thread connects successfully.
				std::shared_ptr<std::mutex> mtx(new std::mutex);
				std::unique_lock lck(*mtx);
				std::shared_ptr<std::condition_variable> ev(
					new std::condition_variable);
				std::shared_ptr<std::unique_ptr<Socket>> socket(
					new std::unique_ptr<Socket>);

				// Spawn multiple Sockets & threads, one for each addressInfo.
				for (std::size_t index = 0; index < addressInfos.size(); index++) {
					// Any specific throw can be consumed safely, but must not crash
					// thread.
					std::thread(
						Rain::Error::consumeThrowable(
							// Can capture this and addressInfo since they are consumed almost
							// immediately.
							[this, mtx, ev, socket, index, &addressInfos, timeout]() {
								// Avoid creating all interruptPairs.
								Socket attemptSocket(this->specification, false);
								if (!attemptSocket.connect(addressInfos[index], timeout)) {
									// Connected! Lock and store this socket, then notify.
									std::lock_guard lckGuard(*mtx);
									if (!*socket) {
										socket->reset(new Socket(std::move(attemptSocket)));
									}
									ev->notify_all();
								}
							}))
						.detach();
				}

				// Wait until notified or timeout.
				if (timeout.isInfinite()) {
					ev->wait(lck, [socket]() { return static_cast<bool>(*socket); });
				} else {
					ev->wait_until(lck, timeout.getTimeoutTime(), [socket]() {
						return static_cast<bool>(*socket);
					});
				}

				// We have re-acquired the mutex. socket will not change. It may change
				// after this function exits, but it is not our problem anymore.
				//
				// If the socket is invalid, return true. Otherwise, swap the internal
				// handles.
				if (!*socket) {
					return true;
				}

				std::swap(this->nativeSocket, (*socket)->nativeSocket);
				return false;
			} else {
				// Try all addresses serially, until timeout.
				std::exception_ptr firstException = nullptr;
				for (Resolve::AddressInfo const &addressInfo : addressInfos) {
					// Errors are caught and discarded. If any error was caught, and none
					// of the connects suceeded, then throw the first error.
					try {
						if (!this->_connect(addressInfo, timeout)) {
							return false;
						}

						// Otherwise, we timed out or errored, so try again in either case.
					} catch (...) {
						if (firstException == nullptr) {
							firstException = std::current_exception();
						}
					}
				}

				if (firstException == nullptr) {
					return true;
				} else {
					std::rethrow_exception(firstException);
				}
			}
		}

		public:
		// connect to a set of AddressInfos. Return false on success, true on
		// timeout or empty addressInfos, throwing the first exception if no address
		// succeeded.
		template <typename Clock = std::chrono::steady_clock>
		bool connect(
			std::vector<Resolve::AddressInfo> const &addressInfos,
			bool parallel = true,
			Time::Timeout<Clock> const &timeout = 60s) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_connect(addressInfos, parallel, timeout);
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		bool _connect(
			Host const &host,
			bool parallel = true,
			Time::Timeout<Clock> const &timeout = 60s,
			// rvalue reference for non-const object with allowed inline
			// initialization.
			Specification::Specification const &gaiSpecification = {},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL) {
			// Get AddressInfos for Host.
			return this->_connect(
				Resolve::getAddressInfo(
					host,
					timeout,
					Specification::Specification(this->specification, gaiSpecification),
					gaiFlags),
				parallel,
				timeout);
		}

		public:
		// connect to a Host with getaddrinfo. Return false on success, true on
		// timeout, throwing the first exception if no address succeeded.
		//
		// addressTimeoutDuration specifies timeout for trying a single resolved
		// address. overallTimeout specifies a timeout for trying all the addresses.
		template <typename Clock = std::chrono::steady_clock>
		bool connect(
			Host const &host,
			bool parallel = true,
			Time::Timeout<Clock> const &timeout = 60s,
			Specification::Specification const &gaiSpecification = {},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_connect(
				host, parallel, timeout, gaiSpecification, gaiFlags);
		}

		private:
		void _bind(Resolve::AddressInfo const &addressInfo) {
			// bind blocks, so no need to poll.
			Error bindError = Error::NONE;
			if (
				::bind(
					this->nativeSocket,
					reinterpret_cast<sockaddr const *>(&addressInfo.address),
					static_cast<socklen_t>(addressInfo.addressLen)) ==
				NATIVE_SOCKET_ERROR) {
				bindError = static_cast<Error>(Networking::getSystemError());
			}

			if (bindError != Error::NONE) {
				throw Networking::Exception(static_cast<Networking::Error>(bindError));
			}
		}

		public:
		// bind to an AddressInfo directly. All errors are thrown; if it returns,
		// bind suceeded.
		void bind(Resolve::AddressInfo const &addressInfo) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			this->_bind(addressInfo);
		}

		private:
		bool _bind(std::vector<Resolve::AddressInfo> const &addressInfos) {
			if (addressInfos.empty()) {
				return true;
			}

			// Try binding each available address. Exceptions except for the first one
			// are caught and discarded. If all addresses fail, the first exception is
			// thrown.
			std::exception_ptr firstException = nullptr;
			for (Resolve::AddressInfo const &addressInfo : addressInfos) {
				try {
					this->_bind(addressInfo);
					return false;
				} catch (...) {
					if (firstException == nullptr) {
						firstException = std::current_exception();
					}
				}
			}

			// Tried all addresses and still didn't return, so throw first exception.
			std::rethrow_exception(firstException);
		}

		public:
		// bind to a vector of AddressInfos.
		bool bind(std::vector<Resolve::AddressInfo> const &addressInfos) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_bind(addressInfos);
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		bool _bind(
			Host const &host = {":0"},
			Time::Timeout<Clock> const &timeout = 60s,
			Specification::Specification const &gaiSpecification = {},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL |
				Resolve::AddressInfoFlag::PASSIVE) {
			// Get AddressInfos for Host.
			return this->_bind(Resolve::getAddressInfo(
				host,
				timeout,
				Specification::Specification(this->specification, gaiSpecification),
				gaiFlags));
		}

		public:
		// bind to a Host via getAddressInfo. Returns false on success, true on
		// getAddressInfo timeout. All other errors are thrown.
		template <typename Clock = std::chrono::steady_clock>
		bool bind(
			Host const &host = {":0"},
			Time::Timeout<Clock> const &timeout = 60s,
			Specification::Specification gaiSpecification = {},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL |
				Resolve::AddressInfoFlag::PASSIVE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_bind(host, timeout, gaiSpecification, gaiFlags);
		}

		// Listen to be used after bind.
		//
		// Very large and very small backlogs are susceptible to SYN flood attacks.
		// Large backlogs may consume resources; small backlogs are not resiliant to
		// small attacks.
		void listen(int backlog = 200) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			this->validateOrThrow(
				::listen(this->nativeSocket, backlog), NATIVE_SOCKET_ERROR);
		}

		private:
		// Private constructor used with accept factory. Uninterruptable by default;
		// interits interruptability in accept.
		Socket(
			NativeSocket nativeSocket,
			Specification::Specification const &specification)
				: nativeSocket(nativeSocket), specification(specification) {}

		public:
		// Accept a new connection on a binded and listening socket. Returns a pair
		// of (Socket, AddressInfo). The Socket is invalid on timeout. Otherwise,
		// Socket is a new valid socket for the connection, and AddressInfo for the
		// accepted peer. Only the address and addressLen fields are valid in the
		// returned AddressInfo.
		//
		// A Socket from accept inherits interruptability properties of this Socket
		// by default.
		template <typename Clock = std::chrono::steady_clock>
		std::pair<Socket, Resolve::AddressInfo> accept(
			Time::Timeout<Clock> const &timeout = 60s) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);

			// Poll until timeout or READ_NORMAL event.
			if (this->poll(PollEvent::READ_NORMAL, timeout) == PollEvent::NONE) {
				// Return an invalid uninterruptable socket on timeout.
				return std::make_pair(
					Socket(NATIVE_SOCKET_INVALID, this->specification),
					Resolve::AddressInfo());
			}

			// If we didn't timeout, then we can accept immediately. The accepted
			// socket has the same specifications as the original.
			Resolve::AddressInfo addressInfo;
			// Temporary socklen_t to be cast to std::size_t later to avoid type
			// punning/strict aliasing violation on addressInfo.addressLen.
			socklen_t addressLen = sizeof(addressInfo.address);
			Socket acceptedSocket(
				this->validateOrThrow(
					::accept(
						this->nativeSocket,
						reinterpret_cast<sockaddr *>(&addressInfo.address),
						&addressLen),
					NATIVE_SOCKET_INVALID),
				this->specification);
			addressInfo.addressLen = static_cast<std::size_t>(addressLen);

			// Accepted sockets need to have their options initialized.
			acceptedSocket.setNonBlocking();
			acceptedSocket.setNoLinger();
			acceptedSocket.interruptPair = this->interruptPair;

			// A lack of move elision will cause compile-time error from deleted copy
			// constructor for Socket.
			return std::make_pair(std::move(acceptedSocket), addressInfo);
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		std::size_t _sendOnce(
			char const *msg,
			std::size_t msgLen,
			Time::Timeout<Clock> const &timeout = 60s,
			SendFlag flags = SendFlag::NONE) {
			// poll until immediately writeable or timeout.
			if (this->poll(PollEvent::WRITE_NORMAL, timeout) == PollEvent::NONE) {
				return 0;
			}

			// No timeout yet, so socket is immediately writeable.
			return static_cast<std::size_t>(this->validateOrThrow(
				::send(
					this->nativeSocket,
#ifdef RAIN_PLATFORM_WINDOWS
					msg,
					static_cast<int>(msgLen),
#else
					reinterpret_cast<const void *>(msg),
					msgLen,
#endif
					static_cast<int>(flags)),
				NATIVE_SOCKET_ERROR));
		}

		template <typename Clock = std::chrono::steady_clock>
		std::size_t _send(
			char const *msg,
			std::size_t msgLen,
			Time::Timeout<Clock> const &timeout = 60s,
			SendFlag flags = SendFlag::NONE) {
			std::size_t bytesSent = 0, sendOnceStatus;
			do {
				sendOnceStatus =
					this->_sendOnce(msg + bytesSent, msgLen - bytesSent, timeout, flags);
				bytesSent += sendOnceStatus;
			} while (bytesSent < msgLen && sendOnceStatus != 0);
			return bytesSent;
		}

		public:
		// sendOnce only triggers poll to block for send, and sends once after that.
		// Useful for when timeout should be specified per-send-idle.
		template <typename Clock = std::chrono::steady_clock>
		std::size_t sendOnce(
			char const *msg,
			std::size_t msgLen,
			Time::Timeout<Clock> const &timeout = 60s,
			SendFlag flags = SendFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_sendOnce(msg, msgLen, timeout, flags);
		}
		template <typename Clock = std::chrono::steady_clock>
		std::size_t sendOnce(
			std::string const &msg,
			Time::Timeout<Clock> const &timeout = 60s,
			SendFlag flags = SendFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_sendOnce(msg.c_str(), msg.length(), timeout, flags);
		}

		// Block until timeout, successfully sent all bytes, or exception. Returns
		// number of bytes sent, or 0 on timeout.
		template <typename Clock = std::chrono::steady_clock>
		std::size_t send(
			char const *msg,
			std::size_t msgLen,
			Time::Timeout<Clock> timeout = Time::Timeout<Clock>(60s),
			SendFlag flags = SendFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_send(msg, msgLen, timeout, flags);
		}
		template <typename Clock = std::chrono::steady_clock>
		std::size_t send(
			std::string const &msg,
			Time::Timeout<Clock> const &timeout = 60s,
			SendFlag flags = SendFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_send(msg.c_str(), msg.length(), timeout, flags);
		}

		private:
		template <typename Clock = std::chrono::steady_clock>
		std::size_t _recv(
			char *buffer,
			std::size_t bufferLen,
			Time::Timeout<Clock> const &timeout = 60s,
			RecvFlag flags = RecvFlag::NONE) {
			// Wait until we are able to read something.
			if (this->poll(PollEvent::READ_NORMAL, timeout) == PollEvent::NONE) {
				return SIZE_MAX;
			}

			int bytesRecv = this->validateOrThrow(
				::recv(
					this->nativeSocket,
#ifdef RAIN_PLATFORM_WINDOWS
					buffer,
					static_cast<int>(bufferLen),
#else
					reinterpret_cast<void *>(buffer),
					bufferLen,
#endif
					static_cast<int>(flags)),
				NATIVE_SOCKET_ERROR);
			return bytesRecv;
		}

		public:
		// Block until recv some bytes. Return SIZE_MAX on timeout, number of
		// bytes received if successful, or 0 if graceful close.
		template <typename Clock = std::chrono::steady_clock>
		std::size_t recv(
			char *buffer,
			std::size_t bufferLen,
			Time::Timeout<Clock> const &timeout = 60s,
			RecvFlag flags = RecvFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			return this->_recv(buffer, bufferLen, timeout, flags);
		}

		// Additionally modifies the buffer to be the size returned, as well as
		// resize beforehand to the capacity of the buffer.
		template <typename Clock = std::chrono::steady_clock>
		std::size_t recv(
			std::string &buffer,
			Time::Timeout<Clock> const &timeout = 60s,
			RecvFlag flags = RecvFlag::NONE) {
			std::lock_guard operationLckGuard(this->operationMtx);
			std::lock_guard stateLckGuard(this->stateMtx);
			buffer.resize(buffer.capacity());
			std::size_t recvStatus =
				this->_recv(&buffer[0], buffer.length(), timeout, flags);
			if (recvStatus != SIZE_MAX) {
				buffer.resize(recvStatus);
			}
			return recvStatus;
		}

		// Issue an interrupt to an interruptPair by sending a byte from both ends,
		// and receiving it as well.
		static void issueInterrupt(
			std::pair<std::shared_ptr<Socket>, std::shared_ptr<Socket>>
				interruptPair) {
			interruptPair.first->send("", 1, Time::Timeout<>());
			// Leave the byte unread to timeout all future polls.
		}

		// Interrupt an ongoing poll using interruptPair. Does not require locking
		// of stateMtx by current thread by snapshotting interruptPair.
		void interrupt() {
			std::lock_guard stateLckGuard(this->stateMtx);
			if (!this->_isInterruptable()) {
				throw Exception(Error::INTERRUPT_ON_UNINTERRUPTABLE);
			} else {
				Socket::issueInterrupt(this->interruptPair);
			}
		}
	};
}

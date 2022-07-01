// Client(Interface) subclasses ConnectedSocket(Interface) with the additional
// requirement to connect to a target Host or AddressInfo on construct.
#pragma once

#include "socket.hpp"

#include <future>

namespace Rain::Networking {
	class ClientSocketSpecInterface
			: virtual public ConnectedSocketSpecInterface {
		protected:
		// Code-sharing: connect any opened NativeSocket to a single AddressInfo.
		// Returns false on success, true on failure.
		//
		// The templates given to ClientSocket do not matter to this function.
		static bool connect(
			NativeSocket nativeSocket,
			AddressInfo const &addressInfo,
			Time::Timeout timeout = 15s) {
			Error connectError{Error::NONE};
			if (
				::connect(
					nativeSocket,
					reinterpret_cast<sockaddr const *>(&addressInfo.address),
					static_cast<socklen_t>(addressInfo.addressLen)) ==
				NATIVE_SOCKET_ERROR) {
				connectError = static_cast<Error>(getSystemError());
			}

			switch (connectError) {
				case Error::NONE:
					return false;
				case Error::IN_PROGRESS:
				case Error::WOULD_BLOCK:
					// Both of these flags arise for non-blocking sockets, and necessitate
					// waiting for connect via poll.
					break;
				default:
					// Unexpected error.
					throw Exception(static_cast<Error>(connectError));
			}

			// connect has returned IN_PROGRESS or WOULD_BLOCK for this non-blocking
			// socket, and we should wait with poll. If returned event is not EXACTLY
			// WRITE_NORMAL, then return failure instead.
			//
			// Note that send/recv are more permissive in their poll return checking;
			// they allow of any combination of flags as long as
			// READ_NORMAL/WRITE_NORMAL is included. It is okay to be as permissive on
			// POSIX-systems for poll after connect, but on Windows, WRITE_NORMAL may
			// return even though an error was triggered and connect failed. Thus, we
			// need to be stricter here, as it does not affect POSIX
			// connected-checking either.
			return SocketInterface::poll(
							 std::vector<NativeSocket>{nativeSocket},
							 {PollFlag::WRITE_NORMAL},
							 timeout)[0] != PollFlag::WRITE_NORMAL;
		}
	};

	// Client(Interface) subclasses ConnectedSocket(Interface) with the additional
	// requirement to connect to a target Host or AddressInfo on construct.
	//
	// This is not a resource-managing class, and should be used after socket
	// options are set on a resource-managing Socket.
	template <typename Socket>
	class ClientSocketSpec : public Socket,
													 virtual public ClientSocketSpecInterface {
		private:
		// Multi-address version of connect. Spawns multiple identical Sockets, each
		// a duplicate of the resource-managed Socket. When any of them connects,
		// swaps it with the current resource-managing Socket. Return false on
		// success, true on failure (all).
		bool connect(
			std::vector<AddressInfo> const &addressInfos,
			Time::Timeout timeout = 15s) {
			std::mutex mtx;

			// Stores all the futures. Must be destructed after lck, since the async
			// functions may use mtx.
			std::vector<std::future<void>> futures;

			// Signalled when any of the futures finish successfully.
			//
			// TODO: If all futures finish unsuccessfully before the timeout, this
			// still waits until the timeout.
			std::unique_lock lck(mtx);
			std::condition_variable ev;

			// mtx locks _nativeSocket. _nativeSocket (managed by superclass) is not
			// directly accessible, so we need to swap it in. This variable determines
			// if the swap has occurred yet (e.g. successfully connected).
			bool connected{false};
			std::atomic_size_t attemptsCompleted{0};

			for (std::size_t idx{0}; idx < addressInfos.size(); idx++) {
				// Each thread spawns an identical client and attempts to connect with
				// that client. On success, locks the mutex and swaps.
				futures.push_back(std::async(
					std::launch::async,
					[this,
					 &mtx,
					 &ev,
					 &connected,
					 &attemptsCompleted,
					 &addressInfo = addressInfos[idx],
					 timeout]() {
						// Ignore any connect exceptions; any exceptions reported during
						// tests probably originates from here.
						Rain::Error::consumeThrowable(
							[this, &mtx, &connected, &addressInfo, timeout]() {
								// Calls the single-address constructor.
								ClientSocketSpec<Socket> attemptSocket(addressInfo, timeout);
								std::lock_guard lck(mtx);
								if (!connected) {
									connected = true;
									this->swap(&attemptSocket);
								}
							})();

						// Fail or succeed, notify that we are done anyway.
						attemptsCompleted++;
						ev.notify_one();
					}));
			}

			// Wait until notified or timeout, or notified that all attempts have
			// failed early.
			auto pred =
				[&connected, &attemptsCompleted, attemptsTotal = futures.size()]() {
					return connected || attemptsCompleted == attemptsTotal;
				};
			if (timeout.isInfinite()) {
				ev.wait(lck, pred);
			} else {
				ev.wait_until(lck, timeout.asTimepoint(), pred);
			}

			return !connected;

			// Now that this->_nativeSocket is set, we need to wait for all the
			// futures to terminate via destructor. this->_nativeSocket will not be
			// modified anymore.
		}

		public:
		// All constructors either connect successfully or throw.
		ClientSocketSpec(
			AddressInfo const &addressInfo,
			Time::Timeout timeout = 15s) {
			if (ClientSocketSpecInterface::connect(
						this->nativeSocket(), addressInfo, timeout)) {
				throw Exception(Error::TIMED_OUT);
			}
		}

		// TODO: Calling any multi-target constructor will cause an unnecessary
		// kernel call to socket (prior to entering the constructor) which will be
		// immediately closed.
		ClientSocketSpec(
			std::vector<AddressInfo> const &addressInfos,
			Time::Timeout timeout = 15s) {
			if (this->connect(addressInfos, timeout)) {
				throw Exception(Error::TIMED_OUT);
			}
		}
		ClientSocketSpec(
			Host const &host,
			Time::Timeout timeout = 15s,
			AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL) {
			// TODO: gai blocks indefinitely! But seems unlikely for many
			// implementations.
			auto addressInfos = getAddressInfos(
				host, this->family(), this->type(), this->protocol(), flags);
			if (this->connect(addressInfos, timeout)) {
				throw Exception(Error::TIMED_OUT);
			}
		}

		// Multi-priority connect: attempts groups in order, with the same timeout
		// for each group.
		ClientSocketSpec(
			std::vector<std::vector<AddressInfo>> const &addressInfoGroups,
			Time::Timeout timeout = 15s) {
			for (auto const &addressInfos : addressInfoGroups) {
				if (!this->connect(addressInfos, timeout)) {
					return;
				}
			}
			throw Exception(Error::TIMED_OUT);
		}
		ClientSocketSpec(
			std::vector<Host> const &hostGroups,
			Time::Timeout timeout = 15s,
			AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL) {
			for (auto const &host : hostGroups) {
				auto addressInfos = getAddressInfos(
					host, this->family(), this->type(), this->protocol(), flags);
				if (!this->connect(addressInfos, timeout)) {
					return;
				}
			}
			throw Exception(Error::TIMED_OUT);
		}
	};

	// Shorthand which includes ConnectedSocket and NamedSocket and base Socket
	// templates.
	template <
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Client
			: public ClientSocketSpec<ConnectedSocketSpec<NamedSocketSpec<Socket<
					SocketFamilyInterface,
					SocketTypeInterface,
					SocketProtocolInterface,
					SocketOptions...>>>> {
		using ClientSocketSpec<ConnectedSocketSpec<NamedSocketSpec<Socket<
			SocketFamilyInterface,
			SocketTypeInterface,
			SocketProtocolInterface,
			SocketOptions...>>>>::ClientSocketSpec;
	};
}

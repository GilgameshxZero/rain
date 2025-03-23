// ServerSocket subclasses NamedSocket(Interface) with the additional
// requirement to bind and listen on construct.
#pragma once

#include "../error/consume-throwable.hpp"
#include "../literal.hpp"
#include "../multithreading/thread-pool.hpp"
#include "../time/timeout.hpp"
#include "client.hpp"
#include "socket.hpp"
#include "worker.hpp"

namespace Rain::Networking {
	// InterfaceInterfaces hold commonalities behind templated Interfaces as well
	// as introduce otherwise dependent names.
	class ServerSocketSpecInterfaceInterface
			: virtual public NamedSocketSpecInterface {
		protected:
		// Code-sharing: bind.
		static void bind(
			NativeSocket nativeSocket,
			AddressInfo const &addressInfo) {
			validateSystemCall(::bind(
				nativeSocket,
				reinterpret_cast<sockaddr const *>(&addressInfo.address),
				static_cast<socklen_t>(addressInfo.addressLen)));
		}
		static void bind(
			NativeSocket nativeSocket,
			std::vector<AddressInfo> const &addressInfos) {
			// Try all addresses in order, unlike Client which is parallel.
			for (AddressInfo const &addressInfo : addressInfos) {
				if (
					::bind(
						nativeSocket,
						reinterpret_cast<sockaddr const *>(&addressInfo.address),
						static_cast<socklen_t>(addressInfo.addressLen)) !=
					NATIVE_SOCKET_ERROR) {
					return;
				}
			}

			// All binds failed.
			throw Exception(getSystemError());
		}
		static void bind(
			NativeSocket nativeSocket,
			Host const &host,
			Family family,
			Type type,
			Protocol protocol,
			AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL |
				AddressInfo::Flag::PASSIVE) {
			ServerSocketSpecInterfaceInterface::bind(
				nativeSocket, getAddressInfos(host, family, type, protocol, flags));
		}

		public:
		virtual std::size_t workers() = 0;
		virtual std::size_t threads() = 0;
	};

	template <typename WorkerSocketSpec>
	class ServerSocketSpecInterface
			: virtual public ServerSocketSpecInterfaceInterface {
		protected:
		// Override to build WorkerSockets.
		virtual WorkerSocketSpec makeWorker(
			NativeSocket nativeSocket,
			SocketInterface *interrupter) = 0;
	};

	// ServerSocket subclasses NamedSocket(Interface) with the additional
	// requirement to bind and listen on construct.
	//
	// This is not a resource-managing class, and should be used after socket
	// options are set on a resource-managing Socket.
	//
	// The template Socket should be a Networking::Socket with a default
	// constructor.
	template <typename WorkerSocketSpec, typename Socket>
	class ServerSocketSpec
			: public Socket,
				virtual public ServerSocketSpecInterface<WorkerSocketSpec>,
				virtual public ServerSocketSpecInterfaceInterface {
		private:
		static std::size_t const LISTEN_BACKLOG_DEFAULT{65535};

		// accept thread and worker threads are spawned with ThreadPool (infinite
		// capacity).
		Multithreading::ThreadPool threadPool;

		// Socket pair created for interrupts.
		std::pair<
			std::unique_ptr<
				Client<Ipv4FamilyInterface, StreamTypeInterface, TcpProtocolInterface>>,
			std::unique_ptr<Networking::Socket<
				Ipv4FamilyInterface,
				StreamTypeInterface,
				TcpProtocolInterface>>>
			interrupter;

		// listen & accept on a bound Socket, in an std::async.
		void serve(int backlog = LISTEN_BACKLOG_DEFAULT) {
			validateSystemCall(::listen(this->nativeSocket(), backlog));

			// Connect the interrupter pair.
			{
				NamedSocketSpec<Networking::Socket<
					Ipv4FamilyInterface,
					StreamTypeInterface,
					TcpProtocolInterface>>
					interrupterServer;
				ServerSocketSpecInterfaceInterface::bind(
					interrupterServer.nativeSocket(),
					{"localhost:0"},
					interrupterServer.family(),
					interrupterServer.type(),
					interrupterServer.protocol());
				validateSystemCall(::listen(interrupterServer.nativeSocket(), 1));
				auto future =
					std::async(std::launch::async, [this, &interrupterServer]() {
						AddressInfo acceptedAddress;
						socklen_t addressLen{sizeof(acceptedAddress.address)};
						interrupterServer.poll(PollFlag::READ_NORMAL);
						this->interrupter.second.reset(
							new Networking::Socket<
								Ipv4FamilyInterface,
								StreamTypeInterface,
								TcpProtocolInterface>(validateSystemCall(::accept(
								interrupterServer.nativeSocket(),
								reinterpret_cast<sockaddr *>(&acceptedAddress.address),
								&addressLen))));

						// Discard the accepted address.
					});
				this->interrupter.first.reset(
					new Client<
						Ipv4FamilyInterface,
						StreamTypeInterface,
						TcpProtocolInterface>(interrupterServer.name()));
			}

			// If no exception, the socket pair is connected. The first is a
			// ClientSocket and is send/recv-able. So, poll with the second with
			// READ_NORMAL, and interrupt by sending on the first.

			// Begin accepting on this server. The server thread must be extremely
			// resilient to exceptions.
			this->threadPool.queueTask([this]() {
				while (true) {
					// If poll throws, it is unlikely the server can be restarted anyway.
					auto flag = SocketInterface::poll(
						{this, this->interrupter.second.get()},
						{PollFlag::READ_NORMAL, PollFlag::READ_NORMAL},
						{})[0];
					if (flag != PollFlag::READ_NORMAL) {
						break;
					}

					NativeSocket nativeSocket{validateSystemCall(
						::accept(this->nativeSocket(), nullptr, nullptr))};

					// Worker must be constructed prior to starting its task, lest the
					// Server deconstruction cause the makeWorker virtual to be
					// unregistered from its subclass and the thread to call the base pure
					// virtual makeWorker.

					// std::function Task cannot store move-captures on unique_ptr.
					try {
						this->threadPool.queueTask(
							Rain::Error::consumeThrowable([this, nativeSocket]() {
								// If Worker construction fails, just consume and ignore the
								// exception. This should be unlikely.
								//
								// This may fail on some platforms if socket options are set
								// and the client disconnects almost immediately, invalidating
								// the file descriptor. Then, socket options constructors will
								// fail on the invalid file descriptor.
								auto worker = this->makeWorker(
									nativeSocket, this->interrupter.second.get());

								// Rate limit based on peer hostname.
								if (!this->shouldRejectPeerHost(worker.peerHost())) {
									// Failures in onWork should be logged.
									Rain::Error::consumeThrowable(
										[&worker]() {
											// Cast to WorkerSocketSpecInterface to trigger friend
											// permissions.
											static_cast<WorkerSocketSpecInterface &>(worker).onWork();
										},
										RAIN_ERROR_LOCATION)();
								}
							}));
					} catch (std::exception const &exception) {
						std::cout << exception.what();

						// Set maxThreads here so that we don’t exceed system maximum.
						// TODO: Is there a better way to determine if we’ve hit the system
						// maximum on threads?
						this->threadPool.setMaxThreads(this->threadPool.getCThreads());
					}
				}
			});
		}

		// Host rate limit sliding window size.
		static std::chrono::steady_clock::duration constexpr RATE_LIMIT_WINDOW_SIZE{
			60s};
		static std::size_t const RATE_LIMIT_THRESHOLD{60};
		// Maps a peer node to a pair (mutex, queue of times) where the queue
		// contains all times the peer has connected within the sliding window,
		// sorted in ascending order. The mutex is used to guarantee thread-safety
		// on the queue.
		// TODO: This map expands over the lifetime of the server and is not cleaned
		// up.
		std::unordered_map<
			std::string,
			std::pair<
				std::mutex,
				std::queue<std::chrono::time_point<std::chrono::steady_clock>>>>
			peerConnections;

		// Implements sliding window rate limiting based on host node (IP).
		virtual bool shouldRejectPeerHost(Host const &peerHost) {
			auto &peerConnection = this->peerConnections[peerHost.node];
			std::lock_guard peerConnectionLck(peerConnection.first);
			auto const now = std::chrono::steady_clock::now();
			while (!peerConnection.second.empty() &&
						 peerConnection.second.front() <= now - RATE_LIMIT_WINDOW_SIZE) {
				peerConnection.second.pop();
			}

			if (peerConnection.second.size() >= RATE_LIMIT_THRESHOLD) {
				return true;
			} else {
				peerConnection.second.push(now);
				return false;
			}
		}

		protected:
		// Subclasses need to wait for worker threads to finish before destructing,
		// since that de-virtualizes their worker constructor override and may cause
		// worker thread to call a pure virtual.
		void destruct() {
			// Send interrupt, then wait for everything to destruct before anything
			// from the class is deconstructed.
			this->interrupter.first->send("\0", 1);
			this->threadPool.blockForTasks();
		}

		public:
		// On construction, Servers bind and listen and accept, asynchronously.
		ServerSocketSpec(
			AddressInfo const &addressInfo,
			int backlog = LISTEN_BACKLOG_DEFAULT) {
			ServerSocketSpecInterfaceInterface::bind(
				this->nativeSocket(), addressInfo);
			this->serve(backlog);
		}
		ServerSocketSpec(
			std::vector<AddressInfo> const &addressInfos,
			int backlog = LISTEN_BACKLOG_DEFAULT) {
			ServerSocketSpecInterfaceInterface::bind(
				this->nativeSocket(), addressInfos);
			this->serve(backlog);
		}
		ServerSocketSpec(
			Host const &host,
			int backlog = LISTEN_BACKLOG_DEFAULT,
			AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL |
				AddressInfo::Flag::PASSIVE) {
			ServerSocketSpecInterfaceInterface::bind(
				this->nativeSocket(),
				host,
				this->family(),
				this->type(),
				this->protocol(),
				flags);
			this->serve(backlog);
		}
		virtual ~ServerSocketSpec() {
			// Most-derived class must call this->destruct().
			// TODO: This is an anti-pattern.
		}

		// Queries.
		virtual std::size_t workers() override {
			return this->threadPool.getCTasks() - 1;
		}
		virtual std::size_t threads() override {
			return this->threadPool.getCThreads();
		}
	};

	// Shorthand which includes NamedSocket and base Socket
	// templates.
	template <
		typename WorkerSocketSpec,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Server : public ServerSocketSpec<
									 WorkerSocketSpec,
									 NamedSocketSpec<Socket<
										 SocketFamilyInterface,
										 SocketTypeInterface,
										 SocketProtocolInterface,
										 SocketOptions...>>> {
		using ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<Socket<
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>::ServerSocketSpec;
	};
}

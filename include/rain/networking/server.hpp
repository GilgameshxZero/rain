// Socket specialization: the templated Server interface, and its protocol
// implementation with the basic Socket.
#pragma once

#include "../error.hpp"
#include "../literal.hpp"
#include "../multithreading/thread-pool.hpp"
#include "../time/timeout.hpp"
#include "socket.hpp"
#include "worker.hpp"

#include <set>

namespace Rain::Networking {
	// Socket specialization: the templated Server interface, and its protocol
	// implementation with the basic Socket.
	//
	// Servers satisfy the same NTA contract as Sockets and ThreadPools, but is
	// NOT thread-safe. A Server whose Workers are NBTA, and only controlled from
	// one thread, is NBTA.
	//
	// The SocketArgs template contains arguments beyond the base protocol Socket
	// (new arguments implemented by the protocol Socket) passed to both the
	// Server Socket and the Worker Sockets. The subclassed Server can define what
	// each of those are in the constructor and workerFactory.
	template <typename ProtocolSocket, typename ProtocolWorker>
	class ServerInterface : public ProtocolSocket {
		public:
		typedef ProtocolSocket Socket;
		typedef ProtocolWorker Worker;
		typedef ServerInterface<Socket, Worker> Interface;

		private:
		// Servers create an interrupt pair on construction, and this must not be
		// altered, as Workers inherit this and depend on it. Interrupts to the
		// server are to be issued via Server close or abort.
		using Socket::setInterruptPair;
		using Socket::setNewInterruptPair;

		// abort and close are overriden on ServerInterface.
		using Socket::abort;
		// Shutdown will throw on Server.
		using Socket::shutdown;
		using Socket::close;
		using Socket::connect;
		using Socket::bind;
		using Socket::listen;
		using Socket::accept;
		using Socket::send;
		using Socket::sendOnce;
		using Socket::recv;
		using Socket::interrupt;

		private:
		// Each worker is given its own Task.
		Multithreading::ThreadPool threadPool;

		std::set<std::unique_ptr<Worker>> workers;
		std::mutex workersMtx;

		// This bit is set when the server has initiated closing to facilitate
		// shutting down the accept thread.
		bool closing = false;

		public:
		// By default, Server is a valid, interruptable IPv6 socket with dual-stack
		// support.
		template <typename... SocketArgs>
		ServerInterface(
			Specification::Specification const &specification =
				{Specification::ProtocolFamily::INET6,
				 Specification::SocketType::STREAM,
				 Specification::SocketProtocol::TCP},
			std::size_t maxThreads = 1024,
			SocketArgs &&...args)
				: Socket(specification, true, std::forward<SocketArgs>(args)...),
					threadPool(maxThreads) {}

		// Block for threads (including accept thread) to exit.
		template <typename Clock = std::chrono::steady_clock>
		bool blockForTasks(Time::Timeout<Clock> const &timeout = {}) {
			return this->threadPool.blockForTasks(timeout);
		}

		// Abort the server, and consumes any exceptions. Then, like any class which
		// utilizes ThreadPool, waits on all threads to exit.
		virtual ~ServerInterface() {
			Rain::Error::consumeThrowable(
				[this]() {
					this->abort();
					this->blockForTasks();
				},
				RAIN_ERROR_LOCATION)();
		}

		// Servers cannot be moved nor copied. Move semantics are not auto-generated
		// by compiler.
		ServerInterface(ServerInterface const &) = delete;
		ServerInterface &operator=(ServerInterface const &) = delete;

		// Attempt to gracefully close the server by first issuing a shutdown, then
		// a graceful close to all Workers. Wait for all threads to exit, up to a
		// timeout, then closes the server. Returns false if all threads exited,
		// true otherwise. A negative timeout blocks indefinitely.
		template <typename Clock = std::chrono::steady_clock>
		bool close(Time::Timeout<Clock> const &timeout = 60s) {
			// Disallow accepting any more clients after the next one. Disable new
			// Tasks.
			this->closing = true;

			// Interrupt all workers, then abort them if they don't themselves after a
			// timeout.
			this->interrupt();

			// Wait for all threads to finish now that all Workers are closed.
			bool threadBlockTimeout = this->blockForTasks(timeout);

			// Abort any remaining workers. This does not ensure that all threads are
			// closed.
			{
				std::lock_guard<std::mutex> workersLckGuard(this->workersMtx);
				for (auto it = this->workers.begin(); it != this->workers.end(); it++) {
					(*it)->abort();
				}
			}

			// Regardless of whether the threads exited gracefully, abort the server.
			Socket::abort();
			return threadBlockTimeout;
		}

		// Abort aggressively all workers and the server, and doesn't wait for
		// threads.
		void abort() {
			this->closing = true;
			this->interrupt();
			{
				std::lock_guard<std::mutex> workersLckGuard(this->workersMtx);
				for (auto it = this->workers.begin(); it != this->workers.end(); it++) {
					(*it)->abort();
				}
			}
			Socket::abort();
		}

		// Bind, listen, and accept continuously until server is closed. Return
		// false on success, true on timeout.
		template <typename Clock = std::chrono::steady_clock>
		bool serve(
			Host const &host = {":0"},
			Time::Timeout<Clock> const &gaiTimeout = 60s,
			typename Clock::duration const &acceptTimeoutDuration = 60s,
			Specification::Specification const &gaiSpecification =
				{Specification::ProtocolFamily::DEFAULT,
				 Specification::SocketType::DEFAULT,
				 Specification::SocketProtocol::DEFAULT},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL |
				Resolve::AddressInfoFlag::PASSIVE,
			int backlog = 200) {
			if (Socket::bind(host, gaiTimeout, gaiSpecification, gaiFlags)) {
				return true;
			}
			this->listen(backlog);

			// The accept loop takes one thread in the ThreadPool.
			this->threadPool.queueTask(Rain::Error::consumeThrowable(
				[this, acceptTimeoutDuration]() {
					// Once the server begins destruction, this function must exit ASAP
					// before the gracefulClose timeout runs out and its member variables
					// are invalidated.
					std::shared_ptr<std::pair<Networking::Socket, Resolve::AddressInfo>>
						acceptRes;
					while (true) {
						// Result from accept is constructed dynamically to be freed later
						// by the worker task.
						//
						// Workers inherit the interruptPair, so they get interrupted
						// with the server.
						acceptRes.reset(
							new std::pair<Networking::Socket, Resolve::AddressInfo>{
								this->accept({acceptTimeoutDuration})});

						// When the server is to be closed, accept will be interrupted. We
						// know this is the case if this->closing is true.
						if (this->closing) {
							// Frees the accepted closeInterruptSocket.
							return;
						}

						if (acceptRes->first.isValid()) {
							// Accepted valid Socket.

							// Queue task in ThreadPool inside of which we will construct the
							// new Worker from the newly minted Socket and run its own task in
							// its own thread. Exceptions in the task are consumed.
							this->threadPool.queueTask([this, acceptRes]() {
								typename std::set<std::unique_ptr<Worker>>::iterator workerIt;

								// Worker behavior is subclassed by the Worker in its
								// constructor.
								{
									std::lock_guard<std::mutex> workersLckGuard(this->workersMtx);
									workerIt =
										this->workers.insert(this->workerFactory(acceptRes)).first;
								}

								// Exceptions in worker are consumed and cerr triggered.
								Rain::Error::consumeThrowable(
									[&workerIt]() {
										// Manually upcast to trigger friend permissions.
										static_cast<WorkerInterface<Socket> *>(workerIt->get())
											->onWork();
									},
									RAIN_ERROR_LOCATION)();

								// Once Worker behavior is completed, it is deleted.
								{
									std::lock_guard<std::mutex> lckGuard(this->workersMtx);
									this->workers.erase(workerIt);
								}
							});
						}

						// accept timeout, just retry.
					}
				},
				RAIN_ERROR_LOCATION));
			return false;
		}

		private:
		// Abstract because cannot infer Worker constructor signature without
		// template.
		//
		// Override this to pass custom arguments to workers during construction.
		//
		// Must satisfy RVO for copy elision or otherwise call std::move.
		virtual std::unique_ptr<Worker> workerFactory(
			std::shared_ptr<std::pair<Networking::Socket, Resolve::AddressInfo>> acceptRes) {
			return std::make_unique<Worker>(
				acceptRes->second, std::move(acceptRes->first));
		}
	};

	typedef ServerInterface<Socket, Worker> Server;
}

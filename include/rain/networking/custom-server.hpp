#pragma once

#include "../thread-pool.hpp"
#include "socket.hpp"

#include <functional>
#include <set>

namespace Rain::Networking {
	// Reference implementation.
	template <typename ServerType, typename SlaveType>
	class CustomServerSlave : protected Socket {
		public:
		// For use in accessing subclass type from superclass.
		typedef ServerType Server;

		Server *server;

		CustomServerSlave(const Socket &socket, Server *server)
				: Socket(socket), server(server) {}
	};

	// Handles listening and accepting of new connections, as well as creating the
	// HttpServerSlave to be configured correctly.
	template <typename SlaveType>
	class CustomServer : protected Socket {
		public:
		// Shorthand.
		typedef SlaveType Slave;

		// Handlers for master/slave lifecycle events. Set these before serving.
		// By default, don't handle any events.
		// Can override by subclassing.
		typedef std::function<void(SlaveType *)> Handler;

		Handler onNewSlave = [](SlaveType *) {};
		Handler onBeginSlaveTask = [](SlaveType *) {};
		Handler onCloseSlave = [](SlaveType *) {};
		Handler onDeleteSlave = [](SlaveType *) {};

		// Slave buffer size must be large enough to store the entire header block
		// of a request.
		CustomServer(std::size_t maxThreads = 0)
				: Socket(true), threadPool(maxThreads) {}

		// Bind, listen, and accept continuously until master is closed.
		void serve(const Host &host, bool blocking = true, int backlog = 1024) {
			// Bind and listen.
			this->bind(host);
			this->listen(backlog);

			std::function<void(void *)> serveSync = [&](void *param) {
				// Continuously accept new connections until master is closed.
				while (true) {
					NativeSocket nativeSocket = NATIVE_SOCKET_INVALID;
					try {
						// Accept will sometimes throw an error when it is broken by close
						// on another thread. If it doesn't, the timeout will keep it in
						// check.
						while (this->getNativeSocket() != NATIVE_SOCKET_INVALID &&
							nativeSocket == NATIVE_SOCKET_INVALID) {
							// 1 minute timeout means that when we close the server, it'll
							// take up to 1 minute to stop blocking for accept/select.
							nativeSocket = this->accept(NULL, NULL, 60000);
						}

						// Terminated by timeout.
						if (nativeSocket == NATIVE_SOCKET_INVALID) {
							break;
						}
					} catch (...) {
						// Accept failed, the server has likely shut down.
						break;
					}
					SlaveType *slave = new SlaveType(Socket(false,
																						 nativeSocket,
																						 this->getFamily(),
																						 this->getType(),
																						 this->getProtocol()),
						reinterpret_cast<typename SlaveType::Server *>(
							this->getSubclassPtr()));
					this->onNewSlave(slave);
					this->slavesMtx.lock();
					this->slaves.insert(slave);
					this->slavesMtx.unlock();

					// For each new connection, spawn a task to handle it.
					this->threadPool.queueTask(
						[this](void *param) {
							SlaveType *slave = reinterpret_cast<SlaveType *>(param);
							this->onBeginSlaveTask(slave);
							this->onCloseSlave(slave);
							this->slavesMtx.lock();
							this->slaves.erase(slave);
							this->slavesMtx.unlock();
							slave->close();
							this->onDeleteSlave(slave);
							delete slave;
						},
						reinterpret_cast<void *>(slave));
				}
			};

			if (blocking) {
				serveSync(NULL);
			} else {
				this->threadPool.queueTask(serveSync, NULL);
			}
		}

		// Closing the server should close all associated slaves as well.
		void close() {
			Socket::close();
			this->slavesMtx.lock();
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				(*it)->close();
			}
			this->slavesMtx.unlock();
		}

		protected:
		ThreadPool threadPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;
		std::mutex slavesMtx;

		// Get the subclass pointer, if there is one.
		// Is this a hack?
		std::function<void *()> getSubclassPtr = [this]() { return this; };
	};
}

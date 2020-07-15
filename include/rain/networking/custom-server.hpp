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
				: Socket(true),
					threadPool(maxThreads) {}

		// Bind, listen, and accept continuously until master is closed.
		int serve(const Host &host,
			bool blocking = true,
			int backlog = 1024) noexcept {
			// Bind and listen.
			int status;
			status = this->bind(host);
			if (status != 0) {
				return status;
			}
			status = this->listen(backlog);
			if (status != 0) {
				return status;
			}

			std::function<void(void *)> serveSync = [&](void *param) {
				// Continuously accept new connections until master is closed.
				while (true) {
					NativeSocket nativeSocket = this->accept();
					if (nativeSocket == NATIVE_SOCKET_INVALID) {
						break;
					}
					SlaveType *slave = new SlaveType(
						Socket(
							false, nativeSocket, this->family, this->type, this->protocol),
						reinterpret_cast<typename SlaveType::Server *>(
							this->getSubclassPtr()));
					this->onNewSlave(slave);
					this->slaves.insert(slave);

					// For each new connection, spawn a task to handle it.
					this->threadPool.queueTask(
						[this](void *param) {
							SlaveType *slave = reinterpret_cast<SlaveType *>(param);
							this->onBeginSlaveTask(slave);
							this->onCloseSlave(slave);
							slave->close();
							this->slaves.erase(slave);
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
			return 0;
		}

		// Closing the server should close all associated slaves as well.
		int close() const noexcept {
			int status;
			status = Socket::close();
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				if ((*it)->close()) {
					status = -1;
				}
			}
			return status;
		}

		protected:
		ThreadPool threadPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;

		// Get the subclass pointer, if there is one.
		// Is this a hack?
		std::function<void *()> getSubclassPtr = [this]() { return this; };
	};
}

#pragma once

#include "../thread-pool.hpp"
#include "socket.hpp"

#include <functional>
#include <set>

namespace Rain::Networking {
	// Reference implementation.
	template <template <typename> class ServerType, typename SlaveType>
	class CustomServerSlave : protected Socket {
		public:
		ServerType<SlaveType> *server;

		CustomServerSlave(const Socket &socket, ServerType<SlaveType> *server)
				: Socket(socket), server(server) {}

		// Public interfaces for protected Socket functions we want to expose.
		int send(const void *msg, std::size_t len, int flags = 0) const noexcept {
			return Socket::send(msg, len, flags);
		}
		int send(const char *msg, int flags = 0) const noexcept {
			return Socket::send(msg, flags);
		}
		int send(const std::string &s, int flags = 0) const noexcept {
			return Socket::send(s, flags);
		}
		int close() const noexcept { return Socket::close(); }
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
					threadPool(new ThreadPool(maxThreads)),
					allocatedThreadPool(true) {}
		CustomServer(ThreadPool *threadPool)
				: Socket(true), threadPool(threadPool), allocatedThreadPool(false) {}
		~CustomServer() {
			if (this->allocatedThreadPool) {
				delete this->threadPool;
			}
		}

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
						this);
					this->onNewSlave(slave);
					this->slaves.insert(slave);

					// For each new connection, spawn a task to handle it.
					this->threadPool->queueTask(
						CustomServer::slaveTask, reinterpret_cast<void *>(slave));
				}
			};

			if (blocking) {
				serveSync(NULL);
			} else {
				this->threadPool->queueTask(serveSync, NULL);
			}
			return 0;
		}

		// Closing the server should close all associated slaves as well.
		int close() const noexcept {
			int status;
			status = Socket::close();
			if (status != 0) {
				return status;
			}
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				status = (*it)->close();
				if (status != 0) {
					return status;
				}
			}
			return 0;
		}

		protected:
		// Used to spawn slaves.
		// `allocated*` is set if we allocated it in the constructor.
		ThreadPool *threadPool;
		bool allocatedThreadPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;

		private:
		// Slave task.
		inline static void slaveTask(void *param) {
			SlaveType *slave = reinterpret_cast<SlaveType *>(param);
			slave->server->onBeginSlaveTask(slave);
			slave->server->onCloseSlave(slave);
			slave->close();
			slave->server->slaves.erase(slave);
			slave->server->onDeleteSlave(slave);
			delete slave;
		};
	};
}

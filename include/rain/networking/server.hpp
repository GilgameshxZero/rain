#pragma once

#include "../thread/.hpp"
#include "./socket.hpp"

#include <functional>
#include <set>

namespace Rain::Networking {
	// Forward declaration.
	template <typename SlaveType>
	class CustomServer;

	// Reference implementation.
	template <typename SlaveType>
	class CustomServerSlave : public Socket {
		public:
		CustomServer<SlaveType> *server;

		CustomServerSlave(Socket socket, CustomServer<SlaveType> *server)
				: Socket(socket), server(server) {}
	};

	// Handles listening and accepting of new connections, as well as creating the
	// HttpServerSlave to be configured correctly.
	template <typename SlaveType>
	class CustomServer : private Socket {
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
		CustomServer(size_t maxThreads = 0)
				: Socket(true),
					threadPool(new Thread::ThreadPool(maxThreads)),
					allocatedThreadPool(true) {}
		CustomServer(Thread::ThreadPool *threadPool)
				: Socket(true),
					threadPool(threadPool),
					allocatedThreadPool(false) {}
		~CustomServer() {
			if (this->allocatedThreadPool) {
				delete this->threadPool;
			}
		}

		// Bind, listen, and accept continuously until master is closed.
		int serve(bool blocking = true,
			Host host = Host(),
			int backlog = 1024) {
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
					if (nativeSocket == INVALID_NATIVE_SOCKET) {
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
						this->slaveTask, reinterpret_cast<void *>(slave));
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
		int close() {
			return Socket::close();
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				(*it)->close();
			}
		}

		protected:
		// Used to spawn slaves.
		// `allocated*` is set if we allocated it in the constructor.
		Thread::ThreadPool *threadPool;
		bool allocatedThreadPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;

		// Slave task.
		std::function<void(void *)> slaveTask = [this](void *param) {
			SlaveType *slave = reinterpret_cast<SlaveType *>(param);
			this->onBeginSlaveTask(slave);
			this->onCloseSlave(slave);
			slave->close();
			this->slaves.erase(slave);
			this->onDeleteSlave(slave);
			delete slave;
		};
	};
}

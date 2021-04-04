// Define custom behavior for Server by subclassing it as in
// `tests/socket-server.cpp`.
#pragma once

#include "../thread-pool.hpp"
#include "socket.hpp"

#include <set>

namespace Rain::Networking {
	// DataType is data carried with each new connected slave socket.
	template <typename SlaveType>
	class Server : virtual protected Socket {
		// Shorthand for use in subclasses defining virtual functions.
		public:
		typedef SlaveType Slave;

		// Subclass behavior.
		protected:
		// Factory: create new slave from accepted Socket.
		virtual Slave *onServerAccept(Socket &acceptedSocket) {
			return new Slave(acceptedSocket);
		}

		// Handler: when new slave is created and ready.
		virtual void onBeginSlaveTask(Slave &) noexcept = 0;

		// Settings.
		private:
		// Slave management.
		ThreadPool<Slave *> threadPool;

		// Active slaves.
		std::set<Slave *> slaves;
		std::mutex slavesMtx;

		// If server is closed, can take up to this much time for thread to stop.
		std::chrono::milliseconds const ACCEPT_TIMEOUT_MS;

		// Getters.
		public:
		using Socket::getFamily;
		using Socket::getNativeSocket;
		using Socket::getProtocol;
		using Socket::getType;
		using Socket::isValid;
		using Socket::getService;

		// Constructor.
		Server(std::size_t maxThreads = 128,
			std::chrono::milliseconds const &ACCEPT_TIMEOUT_MS =
				std::chrono::milliseconds(5000))
				: Socket(),
					threadPool(maxThreads),
					ACCEPT_TIMEOUT_MS(ACCEPT_TIMEOUT_MS) {}
		~Server() {
			// Close can always throw exception if threads execute in weird order.
			try {
				this->close();
			} catch (...) {
			}
		}

		// Close server and slaves and wait for tasks to complete.
		void close() {
			Socket::close();

			// Force-close all slaves and then wait for them to be erased and deleted
			// by corresponding task threads.
			this->slavesMtx.lock();
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				(*it)->close();
			}
			this->slavesMtx.unlock();
			this->threadPool.blockForTasks();
		}

		// Bind, listen, and accept continuously until master is closed.
		void serve(Host const &host,
			bool blocking = true,
			int backlog = SOMAXCONN) {
			this->bind(host);
			this->listen(backlog);
			this->threadPool.queueTask(
				[this](Slave *) {
					// This function needs to exit immediately once the server has been
					// destructed, since its member variables will no longer be valid.
					while (true) {
						try {
							Socket acceptedSocket(
								this->accept(NULL, NULL, this->ACCEPT_TIMEOUT_MS));
							if (acceptedSocket.isValid()) {
								this->slavesMtx.lock();
								Slave *slave =
									*this->slaves.insert(this->onServerAccept(acceptedSocket))
										 .first;
								this->slavesMtx.unlock();

								// For each new connection, spawn a task to handle it.
								this->threadPool.queueTask(
									[this](Slave *slave) {
										this->onBeginSlaveTask(*slave);
										this->slavesMtx.lock();
										this->slaves.erase(slave);
										delete slave;
										this->slavesMtx.unlock();
									},
									slave);
							}
						} catch (...) {
							// Accept exception, server has likely shut down.
							break;
						}
					}
				},
				NULL);

			// Blocking servers need to be closed in another thread.
			if (blocking) {
				this->threadPool.blockForTasks();
			}
		}
	};
}

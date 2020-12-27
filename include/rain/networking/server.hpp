// Define custom behavior for Server by subclassing it as in
// `tests/socket-server.cpp`.
#pragma once

#include "../thread-pool.hpp"
#include "socket.hpp"

#include <functional>
#include <set>

namespace Rain::Networking {
	// Reference implementation.
	template <typename ServerType, typename SlaveType, typename DataType>
	class ServerSlave : virtual protected Socket {
		public:
		// For use in accessing subclass type from superclass.
		typedef ServerType Server;

		Server *server;
		DataType data;

		ServerSlave(Socket &socket, Server *server)
				: Socket(std::move(socket)), server(server) {}

		// Expose some relevant functions from base Socket.
		using Socket::getNativeSocket;
		using Socket::getFamily;
		using Socket::getType;
		using Socket::getProtocol;
		using Socket::send;
		using Socket::recv;
		using Socket::shutdown;
		using Socket::close;
	};

	// Handles listening and accepting of new connections, as well as creating the
	// HttpServerSlave to be configured correctly.
	template <typename SlaveType>
	class Server : virtual protected Socket {
		public:
		// Shorthand.
		typedef SlaveType Slave;
		// If server is closed, can take up to this much time for thread to stop.
		std::chrono::milliseconds acceptTimeoutMs{1000};

		// Subclasses should override this function with their `this` pointer.
		protected:
		virtual void *getSubclassPtr() { return reinterpret_cast<void *>(this); }

		// Handlers.
		virtual void onSlaveTask(SlaveType *) {}

		// Slaves management.
		ThreadPool threadPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;
		std::mutex slavesMtx;

		// Constructor.
		public:
		Server(std::size_t maxThreads = 0) : Socket(), threadPool(maxThreads) {}
		~Server() {
			try {
				this->close();
			} catch (...) {
			}
		}

		// Bind, listen, and accept continuously until master is closed.
		void serve(const Host &host,
			bool blocking = true,
			int backlog = SOMAXCONN) {
			this->bind(host);
			this->listen(backlog);

			// This function needs to exit immediately once the server has been
			// destructed, since its member variables will no longer be valid.
			std::function<void(void *)> serveSync = [this](void *param) {
				// Continuously accept new connections until master is closed.
				while (true) {
					SlaveType *slave = NULL;
					try {
						while (slave == NULL) {
							Socket acceptedSocket(
								this->accept(NULL, NULL, this->acceptTimeoutMs));
							if (acceptedSocket.getNativeSocket() != NATIVE_SOCKET_INVALID) {
								slave = new SlaveType(acceptedSocket,
									reinterpret_cast<typename SlaveType::Server *>(
										this->getSubclassPtr()));
							}
						}
					} catch (...) {
						// Accept exception, server has likely shut down.
						break;
					}
					this->slavesMtx.lock();
					this->slaves.insert(slave);
					this->slavesMtx.unlock();

					// For each new connection, spawn a task to handle it.
					this->threadPool.queueTask(
						[this](void *vpSlave) {
							SlaveType *slave = reinterpret_cast<SlaveType *>(vpSlave);
							this->onSlaveTask(slave);
							this->slavesMtx.lock();
							this->slaves.erase(slave);
							delete slave;
							this->slavesMtx.unlock();
						},
						reinterpret_cast<void *>(slave));
				}
			};

			if (blocking) {
				serveSync(reinterpret_cast<void *>(this));
			} else {
				this->threadPool.queueTask(serveSync, reinterpret_cast<void *>(this));
			}
		}

		// Shutting down the server should shutdown all associated slaves as well.
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

		// Getters.
		Host::Service getService() {
			struct sockaddr_in sin;
			socklen_t addrlen = sizeof(sin);
			getsockname(this->getNativeSocket(),
				reinterpret_cast<struct sockaddr *>(&sin),
				&addrlen);
			return Host::Service(static_cast<std::size_t>(ntohs(sin.sin_port)));
		}

		// Expose some additional getter functions from Socket.
		using Socket::getNativeSocket;
		using Socket::getFamily;
		using Socket::getType;
		using Socket::getProtocol;
	};
}

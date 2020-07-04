#pragma once

#include "./buffer-pool.hpp"
#include "./socket.hpp"
#include "./thread-pool.hpp"

#include <functional>
#include <list>
#include <map>

#include <iostream>

namespace Rain {
	// Forward declarations.
	class HttpClient;
	class HttpServerSocket;

	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class HttpPayload {
		public:
		std::map<std::string, std::string> headers;

		char *body;
	};
	class HttpRequest : public HttpPayload {
		public:
		typedef std::function<int(HttpRequest *)> Handler;

		HttpServerSocket *socket;
	};
	class HttpResponse : public HttpPayload {
		public:
		typedef std::function<int(HttpResponse *)> Handler;

		HttpClient *client;
	};

	// Base class for HttpClient and HttpServerSlave.
	class HttpSocket : public Socket {
		public:
		HttpSocket(Socket socket) : Socket(socket) {}
	};
	class HttpClient : public HttpSocket {};

	// Forward declaration.
	template <typename SlaveType>
	class CustomHttpServer;

	// Reference implementation.
	template <typename SlaveType>
	class CustomHttpServerSlave : public HttpSocket {
		public:
		CustomHttpServer<SlaveType> *server;

		CustomHttpServerSlave(Socket socket, CustomHttpServer<SlaveType> *server)
				: HttpSocket(socket), server(server) {}
	};

	// Handles listening and accepting of new connections, as well as creating the
	// HttpServerSlave to be configured correctly.
	template <typename SlaveType>
	class CustomHttpServer : private Socket {
		public:
		// Handlers for master/slave lifecycle events. Set these before serving.
		// By default, don't handle any events.
		// Can override by subclassing.
		typedef std::function<void(SlaveType *)> Handler;

		Handler onNew = [](SlaveType *) {};
		Handler onTask = [](SlaveType *) {};
		HttpRequest::Handler onRequest = [](HttpRequest *) { return 0; };
		Handler onClose = [](SlaveType *) {};
		Handler onDelete = [](SlaveType *) {};

		// Slave buffer size must be large enough to store the entire header block
		// of a request.
		CustomHttpServer(size_t maxThreads = 0, size_t slaveBufSz = 16384)
				: Socket(),
					threadPool(new ThreadPool(maxThreads)),
					allocatedThreadPool(true),
					bufferPool(new BufferPool(slaveBufSz)),
					allocatedBufferPool(true) {}
		CustomHttpServer(ThreadPool *threadPool, BufferPool *bufferPool)
				: Socket(),
					threadPool(threadPool),
					allocatedThreadPool(false),
					bufferPool(bufferPool),
					allocatedBufferPool(false) {}

		// Bind, listen, and accept continuously until master is closed.
		int serve(bool blocking = true,
			const char *service = "80",
			int backlog = 1024) {
			// Bind and listen.
			int status;
			status = this->bind(service);
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
					Socket::NativeSocket nativeSocket = this->accept();
					if (nativeSocket == Socket::INVALID_NATIVE_SOCKET) {
						break;
					}
					SlaveType *slave = new SlaveType(
						Socket(nativeSocket, this->family, this->type, this->protocol),
						this);
					this->onNew(slave);

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

		// Passthrough of some private Socket functions.
		int close() { return Socket::close(); }

		private:
		// Used to spawn slaves.
		ThreadPool *threadPool;
		bool allocatedThreadPool;
		BufferPool *bufferPool;
		bool allocatedBufferPool;

		// Slave task.
		std::function<void(void *)> slaveTask = [this](void *param) {
			SlaveType *slave = reinterpret_cast<SlaveType *>(param);
			this->onTask(slave);

			// Continuously accept requests and call onRequest when we have
			// enough information.
			while (true) {
				// Parse until we have entire header.
				slave->recv(NULL, 0);
				this->onRequest(NULL);
				break;
			}

			this->onClose(slave);
			slave->close();
			this->onDelete(slave);
			delete slave;
		};
	};

	// Non-templated reference subclassing implementation.
	class HttpServerSlave : public CustomHttpServerSlave<HttpServerSlave> {
		public:
		HttpServerSlave(Socket socket, CustomHttpServer<HttpServerSlave> *server)
				: CustomHttpServerSlave<HttpServerSlave>(socket, server) {}
	};
	typedef CustomHttpServer<HttpServerSlave> HttpServer;
}

#pragma once

#include "./socket.hpp"
#include "./thread-pool.hpp"
#include "./time.hpp"

#include <list>

#include <iostream>

namespace Rain {
	class HttpServerSocket : public Socket {
		public:
		// Receiving buffer for header information and body. Must be large enough to
		// contain entire header.
		size_t bufSz;
		char *buf;

		HttpServerSocket(Socket socket = Socket(), size_t bufSz = 16384)
				: Socket(socket), bufSz(bufSz) {
			this->buf = new char[this->bufSz];
		}
		~HttpServerSocket() { delete this->buf; }

		// Accept on an HttpServerSocket creates another HttpServerSocket.
		HttpServerSocket accept(struct sockaddr *addr = NULL,
			AddressLength *addrLen = NULL) {
			return HttpServerSocket(Socket(::accept(this->socket, addr, addrLen),
																this->family,
																this->type,
																this->protocol),
				this->bufSz);
		}

		// Shorthand for bind, listen, and looping accept. Break only when the
		// current socket is shut down.
		int serve(const char *service, ThreadPool *threadPool, int backlog = 1024) {
			int status;

			status = Socket::bind(service);
			if (status != 0) {
				return status;
			}

			status = Socket::listen(backlog);
			if (status != 0) {
				return status;
			}

			// Continuously accept new connections.
			while (true) {
				HttpServerSocket *socket = new HttpServerSocket(this->accept());
				if (socket->socket == 0) {
					return 0;
				}

				// For each new connection, spawn a task to handle it.
				threadPool->queueTask(
					[](void *param) {
						HttpServerSocket *socket =
							reinterpret_cast<HttpServerSocket *>(param);
						std::cout << "spawned socket" << std::endl;
						socket->send("HTTP/1.1 200 OK\r\n");
						socket->send("content-type: text/html; charset=UTF-8\r\n");
						socket->send("server: emilia-web\r\n");
						socket->send("content-length: 10\r\n");
						socket->send("\r\n");
						socket->send("i love you\r\n");
						sleep(500);
						std::cout << "killed socket" << std::endl;
						socket->close();
						delete socket;
					},
					reinterpret_cast<void *>(socket));
			}
		}
	};
}

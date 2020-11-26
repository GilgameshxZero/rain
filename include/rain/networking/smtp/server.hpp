// SMTP server implementation.
#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// Forward declaration.
	template <typename DataType = void *>
	class Server;

	template <typename DataType>
	class ServerSlave : public Smtp::Socket,
											public CustomServerSlave<Smtp::Server<DataType>,
												Smtp::ServerSlave<DataType>,
												DataType> {
		public:
		typedef CustomServerSlave<Smtp::Server<DataType>,
			Smtp::ServerSlave<DataType>,
			DataType>
			CustomServerSlaveBase;

		// Buffer used internally by Server.
		std::size_t bufSz = 0;
		char *buf = NULL;

		ServerSlave(const Networking::Socket &socket,
			Smtp::Server<DataType> *server)
				: Smtp::Socket(socket), CustomServerSlaveBase(socket, server) {}

		// Public interfaces for protected functions we want to expose.
		void send(Request *req) const { Smtp::Socket::send(req); }
		void send(Response *res) const { Smtp::Socket::send(res); }
		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlaveBase::send(msg, len, flags, timeoutMs);
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlaveBase::send(s, flags, timeoutMs);
		}
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlaveBase::recv(buf, len, flags, timeoutMs);
		}
		void close() { CustomServerSlaveBase::close(); }
		NativeSocket getNativeSocket() {
			return CustomServerSlaveBase::getNativeSocket();
		}
	};

	template <typename DataType>
	class Server : private Smtp::Socket,
								 private CustomServer<Smtp::ServerSlave<DataType>> {
		public:
		// Shorthand.
		typedef Smtp::ServerSlave<DataType> Slave;
		typedef CustomServer<Slave> CustomServerBase;

		// Request and response with a slave socket.
		class Payload {
			public:
			Slave *slave;

			Payload(Slave *slave = NULL) : slave(slave) {}
		};
		class Request : public Payload, public Smtp::Request {
			public:
			typedef std::function<void(Request *)> Handler;

			Request(Slave *slave) : Server::Payload(slave), Smtp::Request("", "") {}
			void send() { this->slave->send(this); }
		};
		class Response : public Payload, public Smtp::Response {
			public:
			typedef std::function<void(Response *)> Handler;

			Response(Slave *slave,
				std::size_t code = 250,
				const std::string &parameter = "")
					: Server::Payload(slave), Smtp::Response(code, parameter) {}
			void send() { this->slave->send(this); }
		};

		// Default handlers.
		typename CustomServerBase::Handler onRequestOverflow = [](Slave *slave) {
			Rain::Networking::Smtp::Response res(500, "request too long");
			slave->send(&res);
		};
		typename Request::Handler onRequest = [](Request *) {};
		typename CustomServerBase::Handler onNewSlave = [](Slave *) {};
		typename CustomServerBase::Handler onBeginSlaveTask = [](Slave *) {};
		typename CustomServerBase::Handler onCloseSlave = [](Slave *) {};
		typename CustomServerBase::Handler onDeleteSlave = [](Slave *) {};

		// Max amount of time to wait on each recv.
		std::size_t recvTimeoutMs = 5000;

		// Constructor and destructor. Socket is initialized in CustomServer.
		Server(std::size_t maxThreads = 0, std::size_t slaveBufSz = 16384)
				: Smtp::Socket(), CustomServerBase(maxThreads), slaveBufSz(slaveBufSz) {
			CustomServerBase::onNewSlave = [](Slave *slave) {
				slave->server->onNewSlave(slave);
			};
			CustomServerBase::onBeginSlaveTask = handleBeginSlaveTask;
			CustomServerBase::onCloseSlave = [](Slave *slave) {
				slave->server->onCloseSlave(slave);
			};
			CustomServerBase::onDeleteSlave = [](Slave *slave) {
				slave->server->onDeleteSlave(slave);
			};

			CustomServerBase::getSubclassPtr = [this]() { return this; };
		}
		~Server() {
			// This subclass gets destructed before superclass if we don't block for
			// all tasks.
			CustomServerBase::threadPool.blockForTasks();
		}

		// Expose interfaces of CustomServer.
		std::size_t &acceptTimeoutMs = CustomServerBase::acceptTimeoutMs;

		void serve(const Host &host, bool blocking = true, int backlog = 1024) {
			CustomServerBase::serve(host, blocking, backlog);
		}
		void close() { CustomServerBase::close(); }
		Host::Service getService() { return CustomServerBase::getService(); }

		protected:
		// Size of header-parse and body buffers in each of the slaves.
		const std::size_t slaveBufSz;

		// Smtp parsing and response handling.
		inline static void handleBeginSlaveTask(Slave *slave) {
			slave->server->onBeginSlaveTask(slave);

			// Buffer for header & body parsing.
			slave->bufSz = slave->server->slaveBufSz;
			slave->buf = new char[slave->bufSz];

			// Continuously accept requests and call onRequest when we have enough
			// information.
			while (true) {
				Request req(slave);
				try {
					if (parseRequest(req)) {
						break;
					}
				} catch (...) {
					// If recv dies during request header parsing, just kill the socket.
					break;
				}

				// TODO: Pass onto other handlers.
				try {
					slave->server->onRequest(&req);
				} catch (...) {
					// Any error while handling request should just close connection.
					break;
				}
			}

			// Free slave buffer space.
			delete[] slave->buf;
		}

		// Given a slave socket, wait until the socket is closed or we have parsed a
		// request. Returns nonzero if error.
		inline static int parseRequest(Request &req) {
			// Constants for KMP.
			static const char *CRLF = "\r\n";
			static const std::size_t PART_MATCH_CRLF[] = {
				(std::numeric_limits<std::size_t>::max)(), 0, 0};

			// Parse the entire header.
			// State of newline search.
			std::size_t kmpCand = 0;
			char *curRecv = req.slave->buf;	 // Last search position.

			// Keep on calling recv until we find \r\n.
			while (true) {
				// Is the buffer full? If so, we can't receive the rest of the
				// header, so need to handle that.
				std::size_t bufRemaining =
					req.slave->bufSz - (curRecv - req.slave->buf);
				if (bufRemaining == 0) {
					req.slave->server->onRequestOverflow(req.slave);
					return -1;
				}

				// Receive the next set of bytes from the slave.
				std::size_t recvLen = req.slave->recv(curRecv,
					bufRemaining,
					RecvFlag::NONE,
					req.slave->server->recvTimeoutMs);
				if (recvLen == 0) {	 // Graceful exit.
					return -1;
				}

				// Parsing the request line. Look for end-of-line.
				char *newline = Algorithm::cStrSearchKMP(
					curRecv, recvLen, CRLF, 2, PART_MATCH_CRLF, &kmpCand);
				if (newline != NULL) {
					// Found newline; create request.
					char *space = req.slave->buf;
					for (; space != newline && *space != ' '; space++)
						;
					req.verb.assign(req.slave->buf, space - req.slave->buf);
					if (space + 1 < newline) {
						req.parameter.assign(space + 1, newline - space - 1);
					}
					break;
				}

				curRecv += recvLen;
			}

			return 0;
		}
	};
}

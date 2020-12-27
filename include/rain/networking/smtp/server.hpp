// SMTP server implementation.
#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// Both base classes virtually inherit Socket, so there will only be one copy.
	template <typename ServerType, typename SlaveType, typename DataType>
	class ServerSlave
			: public Networking::ServerSlave<ServerType, SlaveType, DataType>,
				public Smtp::Socket {
		public:
		typedef Networking::ServerSlave<ServerType, SlaveType, DataType>
			ServerSlaveBase;
		ServerSlave(Networking::Socket &socket, ServerType *server)
				: Networking::Socket(std::move(socket)),
					ServerSlaveBase(socket, server),
					Smtp::Socket(socket) {}

		// Buffer used internally by Slave.
		std::size_t bufSz = 0;
		char *buf = NULL;

		// Ambiguity resolution via exposure.
		using ServerSlaveBase::send;
		using Smtp::Socket::send;
	};

	template <typename SlaveType>
	class Server : public Networking::Server<SlaveType>, protected Http::Socket {
		public:
		// Shorthand.
		typedef Networking::Server<SlaveType> ServerBase;

		// If server is closed, can take up to this much time for threads to stop.
		std::chrono::milliseconds recvTimeoutMs{1000};

		// Request and response with a slave socket.
		class Payload {
			public:
			SlaveType *slave;

			Payload(SlaveType *slave = NULL) : slave(slave) {}
		};
		class Request : public Payload, public Smtp::Request {
			public:
			typedef std::function<void(Request *)> Handler;

			Request(SlaveType *slave) : Server::Payload(slave), Smtp::Request("", "") {}
			void send() { this->slave->send(this); }
		};
		class Response : public Payload, public Smtp::Response {
			public:
			typedef std::function<void(Response *)> Handler;

			Response(SlaveType *slave,
				std::size_t code = 250,
				const std::string &parameter = "")
					: Server::Payload(slave), Smtp::Response(code, parameter) {}
			void send() { this->slave->send(this); }
		};

		// Default handlers.
		protected:
		virtual void *getSubclassPtr() { return reinterpret_cast<void *>(this); }
		virtual void onBeginSlaveTask(SlaveType *) {}
		void onSlaveTask(SlaveType *slave) {
			this->onBeginSlaveTask(slave);
			
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
					this->onRequest(&req);
				} catch (...) {
					// Any error while handling request should just close connection.
					break;
				}
			}

			// Free slave buffer space.
			delete[] slave->buf;
		}
		virtual void onRequestOverflow(SlaveType *slave) {
			Rain::Networking::Smtp::Response res(500, "request too long");
			slave->send(&res);
		}
		virtual void onRequest(Request *) {}

		// Size of header-parse and body buffers in each of the slaves.
		const std::size_t slaveBufSz;

		// Constructor.
		public:
		Server(std::size_t maxThreads = 0, std::size_t slaveBufSz = 16384)
				: ServerBase(maxThreads), slaveBufSz(slaveBufSz) {}

		protected:
		// Given a slave socket, wait until the socket is closed or we have parsed a
		// request. Returns nonzero if error.
		int parseRequest(Request &req) {
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
					this->recvTimeoutMs);
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

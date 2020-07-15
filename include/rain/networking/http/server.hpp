#pragma once

#include "../../algorithm/kmp.hpp"
#include "../custom-server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	// Forward declaration.
	class Server;

	class ServerSlave
			: protected Http::Socket,
				public CustomServerSlave<Http::Server, Http::ServerSlave> {
		public:
		// Buffer used internally by Server.
		std::size_t bufSz = 0;
		char *buf = NULL;

		ServerSlave(const Networking::Socket &socket, Http::Server *server)
				: Http::Socket(socket), CustomServerSlave(socket, server) {}

		// Public interfaces for protected functions we want to expose.
		int send(Request *req) { return Http::Socket::send(req); }
		int send(Response *res) { return Http::Socket::send(res); }
		int send(const void *msg, std::size_t len, int flags = 0) const noexcept {
			return CustomServerSlave::send(msg, len, flags);
		}
		int send(const char *msg, int flags = 0) const noexcept {
			return CustomServerSlave::send(msg, flags);
		}
		int send(const std::string &s, int flags = 0) const noexcept {
			return CustomServerSlave::send(s, flags);
		}
		int recv(void *buf, std::size_t len, int flags = 0) const noexcept {
			return Http::Socket::recv(buf, len, flags);
		}
		int close() const noexcept { return Http::Socket::close(); }
	};

	class Server : private Http::Socket, private CustomServer<Http::ServerSlave> {
		public:
		// Shorthand.
		typedef Http::ServerSlave Slave;

		// Request and response with a slave socket.
		class Payload {
			public:
			Slave *slave;

			Payload(Slave *slave = NULL) : slave(slave) {}
		};
		class Request : public Payload, public Http::Request {
			public:
			typedef std::function<void(Request *)> Handler;

			Request(Slave *slave)
					: Server::Payload(slave), Http::Request("", "", "") {}

			int send() { return this->slave->send(this); }
		};
		class Response : public Payload, public Http::Response {
			public:
			typedef std::function<void(Response *)> Handler;

			Response(Slave *slave,
				std::size_t statusCode = 200,
				const std::string &status = "OK",
				const std::string &version = "1.1")
					: Server::Payload(slave), Http::Response() {}

			int send() { return this->slave->send(this); }
		};

		// Default handlers.
		CustomServer::Handler onHeaderOverflow = [](Slave *slave) {
			Response(slave, 413, "Header overflowed", "1.1").send();
		};
		Request::Handler onRequest = [](Request *) {};

		// Expose some handlers of the CustomServer.
		CustomServer::Handler onNewSlave = [](Slave *) {};
		CustomServer::Handler onBeginSlaveTask = [](Slave *) {};
		CustomServer::Handler onCloseSlave = [](Slave *) {};
		CustomServer::Handler onDeleteSlave = [](Slave *) {};

		// Constructor and destructor. Socket is initialized in CustomServer.
		Server(std::size_t maxThreads = 0, std::size_t slaveBufSz = 16384)
				: Http::Socket(), CustomServer(maxThreads), slaveBufSz(slaveBufSz) {
			CustomServer::onNewSlave = [](Slave *slave) {
				slave->server->onNewSlave(slave);
			};
			CustomServer::onBeginSlaveTask = handleBeginSlaveTask;
			CustomServer::onCloseSlave = [](Slave *slave) {
				slave->server->onCloseSlave(slave);
			};
			CustomServer::onDeleteSlave = [](Slave *slave) {
				slave->server->onDeleteSlave(slave);
			};
			CustomServer::getSubclassPtr = [this]() { return this; };
		}
		~Server() {
			// This subclass gets destructed before superclass if we don't block for
			// all tasks.
			CustomServer::threadPool.blockForTasks();
		}

		// Expose interfaces of CustomServer.
		int serve(const Host &host,
			bool blocking = true,
			int backlog = 1024) noexcept {
			return CustomServer::serve(host, blocking, backlog);
		}
		int close() const noexcept { return CustomServer::close(); }

		protected:
		// Size of header-parse and body buffers in each of the slaves.
		const std::size_t slaveBufSz;

		// Http parsing and response handling.
		inline static void handleBeginSlaveTask(Slave *slave) {
			slave->server->onBeginSlaveTask(slave);

			// Buffer for header & body parsing.
			slave->bufSz = slave->server->slaveBufSz;
			slave->buf = new char[slave->bufSz];

			// Continuously accept requests and call onRequest when we have enough
			// information.
			while (true) {
				Request req(slave);
				if (parseRequest(req)) {
					break;
				}

				// TODO: Pass onto other handlers.
				slave->server->onRequest(&req);
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
			char *curRecv = req.slave->buf,	 // Where we're recv-ing into.
				*curParse =
					req.slave->buf,	 // Where we're parsing (not head of curRecv).
					*startOfLine =
						req.slave->buf,	 // The first character past the previous \r\n.
						*endOfLine = NULL;	// The \r of the next \r\n.

			// Keep on calling recv until we're done.
			while (startOfLine != endOfLine) {
				// Is the buffer full? If so, we can't receive the rest of the
				// header, so need to handle that.
				std::size_t bufRemaining =
					req.slave->bufSz - (curParse - req.slave->buf);
				if (bufRemaining == 0) {
					req.slave->server->onHeaderOverflow(req.slave);
					return 1;
				}

				// Receive the next set of bytes from the slave.
				int recvLen =
					req.slave->recv(reinterpret_cast<void *>(curRecv), bufRemaining);
				if (recvLen <= 0) {
					// Graceful or error.
					return 1;
				}
				curRecv += recvLen;

				// Parse everything we just received.
				while (true) {
					// Parsing the request line. Look for end-of-line.
					endOfLine = Algorithm::cStrSearchKMP(
						curParse, curRecv - curParse, CRLF, 2, PART_MATCH_CRLF, &kmpCand);
					if (endOfLine == NULL) {
						// No end of line, so wait for the next recv.
						curParse = curRecv;
						break;
					}
					if (startOfLine == endOfLine) {	 // Empty line, we are done.
						break;
					}

					// Some end of line found.
					curParse = endOfLine + 2;

					// Determine what we're parsing based on what fields are filled
					// out from the request.
					if (req.method.length() == 0) {	 // Looking for request line.
						// Method is the first token before space.
						char *firstSpace;
						for (firstSpace = startOfLine;
								 *firstSpace != ' ' && firstSpace != endOfLine;
								 firstSpace++)
							;
						req.method.assign(startOfLine, firstSpace - startOfLine);

						// Version is the last token after a slash.
						char *lastSlash;
						for (lastSlash = endOfLine;
								 *lastSlash != '/' && lastSlash - 5 != firstSpace;
								 lastSlash--)
							;
						req.version.assign(lastSlash + 1, endOfLine - 1 - lastSlash);

						// URI is stuff in between.
						req.uri.assign(firstSpace + 1, lastSlash - 6 - firstSpace);
					} else {	// Looking for header block.
						// Look for first colon.
						char *firstColon;
						for (firstColon = startOfLine;
								 *firstColon != ':' && firstColon != endOfLine;
								 firstColon++)
							;

						// Normalize and add to header.
						*firstColon = '\0';
						startOfLine[1 +
							String::findFirstNonWhitespaceCStrN(
								firstColon - 1, firstColon - startOfLine, -1) -
							startOfLine] = '\0';
						firstColon[1 +
							String::findFirstNonWhitespaceCStrN(
								endOfLine - 1, endOfLine - firstColon - 1, -1) -
							firstColon] = '\0';
						req.header[std::string(String::findFirstNonWhitespaceCStrN(
							startOfLine, firstColon - startOfLine))] =
							std::string(String::findFirstNonWhitespaceCStrN(
								firstColon + 1, endOfLine - firstColon - 1));
					}

					// Mark the start of the next line.
					startOfLine = endOfLine + 2;
				}
			}

			// Setup body retrieval. The body we have right now starts at endOfLine +
			// 2 and goes until curRecv.
			req.body.appendBytes(endOfLine + 2, curRecv - endOfLine - 2);
			req.body.appendGenerator(getRequestBodyGenerator(req,
				std::strtoull(req.header["Content-Length"].c_str(), NULL, 10) -
					(curRecv - endOfLine - 2)));

			return 0;
		}

		// When called with a clear buffer, will recv enough bytes from slave and
		// send to getBody caller.
		inline static Body::Generator getRequestBodyGenerator(Request &req,
			std::size_t contentLength) {
			if (contentLength == 0) {
				return [](char **) { return 0; };
			}
			return [&req, contentLength](char **bytes) {
				std::size_t bodyLen = static_cast<std::size_t>(req.slave->recv(
					reinterpret_cast<void *>(req.slave->buf), req.slave->bufSz));
				*bytes = req.slave->buf;
				req.body.appendGenerator(
					getRequestBodyGenerator(req, contentLength - bodyLen));
				return bodyLen;
			};
		}
	};
}

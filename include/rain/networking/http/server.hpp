#pragma once

#include "../../algorithm/kmp.hpp"
#include "../custom-server.hpp"
#include "socket.hpp"

#include <iostream>

namespace Rain::Networking::Http {
	// Forward declaration.
	class Server;

	class ServerSlave
			: public Http::Socket,
				public CustomServerSlave<Http::Server, Http::ServerSlave> {
		public:
		// Buffer used internally by Server.
		std::size_t bufSz = 0;
		char *buf = NULL;

		ServerSlave(const Networking::Socket &socket, Http::Server *server)
				: Http::Socket(socket), CustomServerSlave(socket, server) {}

		// Public interfaces for protected functions we want to expose.
		void send(Request *req) const { Http::Socket::send(req); }
		void send(Response *res) const { Http::Socket::send(res); }
		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(msg, len, flags, timeoutMs);
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(s, flags, timeoutMs);
		}
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::recv(buf, len, flags, timeoutMs);
		}
		void close() { CustomServerSlave::close(); }
		NativeSocket getNativeSocket() {
			return CustomServerSlave::getNativeSocket();
		}
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

			void send() { this->slave->send(this); }
		};
		class Response : public Payload, public Http::Response {
			public:
			typedef std::function<void(Response *)> Handler;

			Response(Slave *slave,
				std::size_t statusCode = 200,
				const std::string &status = "OK",
				const std::string &version = "1.1")
					: Server::Payload(slave),
						Http::Response(statusCode, status, version) {}

			void send() { this->slave->send(this); }
		};

		// Default handlers.
		CustomServer::Handler onHeaderOverflow = [](Slave *slave) {
			Response(slave, 413, "Header overflowed", "1.1").send();
		};
		Request::Handler onRequest = [](Request *) {};

		// Some handlers of the CustomServer.
		CustomServer::Handler onNewSlave = [](Slave *) {};
		CustomServer::Handler onBeginSlaveTask = [](Slave *) {};
		CustomServer::Handler onCloseSlave = [](Slave *) {};
		CustomServer::Handler onDeleteSlave = [](Slave *) {};

		// Max amount of time to wait on each recv.
		std::size_t recvTimeoutMs = 5000;

		std::size_t &acceptTimeoutMs = CustomServer::acceptTimeoutMs;

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
		void serve(const Host &host, bool blocking = true, int backlog = 1024) {
			CustomServer::serve(host, blocking, backlog);
		}
		void close() { CustomServer::close(); }

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
					return -1;
				}

				// Receive the next set of bytes from the slave, with a timeout of 1
				// minute.
				std::size_t recvLen = req.slave->recv(curRecv,
					bufRemaining,
					RecvFlag::NONE,
					req.slave->server->recvTimeoutMs);
				if (recvLen == 0) {	 // Graceful exit.
					return -1;
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
								 *firstSpace != ' ' && firstSpace < endOfLine;
								 firstSpace++)
							;
						req.method.assign(startOfLine, firstSpace - startOfLine);

						// Version is the last token after a slash.
						char *lastSlash;
						for (lastSlash = endOfLine;
								 *lastSlash != '/' && lastSlash - 5 > firstSpace;
								 lastSlash--)
							;
						req.version.assign(lastSlash + 1, endOfLine - 1 - lastSlash);

						// path, query, fragment is the stuff in between.
						char *queryQuestionMark;
						for (queryQuestionMark = firstSpace;
								 *queryQuestionMark != '?' && queryQuestionMark < lastSlash - 5;
								 queryQuestionMark++)
							;
						req.path.assign(firstSpace + 1, queryQuestionMark - firstSpace - 1);
						char *fragmentPound;
						for (fragmentPound = queryQuestionMark;
								 *fragmentPound != '#' && fragmentPound < lastSlash - 5;
								 fragmentPound++)
							;
						req.query.assign(
							queryQuestionMark, fragmentPound - queryQuestionMark);
						req.fragment.assign(fragmentPound, lastSlash - fragmentPound - 5);
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
			// Throws exception on error.
			return [&req, contentLength](char **bytes) {
				std::size_t bodyLen =
					static_cast<std::size_t>(req.slave->recv(req.slave->buf,
						req.slave->bufSz,
						RecvFlag::NONE,
						req.slave->server->recvTimeoutMs));
				*bytes = req.slave->buf;
				req.body.appendGenerator(
					getRequestBodyGenerator(req, contentLength - bodyLen));
				return bodyLen;
			};
		}
	};
}

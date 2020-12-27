#pragma once

#include "../../algorithm/kmp.hpp"
#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	// Both base classes virtually inherit Socket, so there will only be one copy.
	template <typename ServerType, typename SlaveType, typename DataType>
	class ServerSlave
			: public Networking::ServerSlave<ServerType, SlaveType, DataType>,
				public Http::Socket {
		public:
		typedef Networking::ServerSlave<ServerType, SlaveType, DataType>
			ServerSlaveBase;
		ServerSlave(Networking::Socket &socket, ServerType *server)
				: Networking::Socket(std::move(socket)),
					ServerSlaveBase(socket, server),
					Http::Socket(socket) {}

		// Buffer used internally by Slave.
		std::size_t bufSz = 0;
		char *buf = NULL;

		// Ambiguity resolution via exposure.
		using ServerSlaveBase::send;
		using Http::Socket::send;
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
		class Request : public Payload, public Http::Request {
			public:
			typedef std::function<void(Request *)> Handler;

			Request(SlaveType *slave)
					: Server::Payload(slave), Http::Request("", "", "") {}
			void send() { this->slave->send(this); }
		};
		class Response : public Payload, public Http::Response {
			public:
			typedef std::function<void(Response *)> Handler;

			Response(SlaveType *slave,
				std::size_t statusCode = 200,
				const std::string &status = "OK",
				const std::string &version = "1.1")
					: Server::Payload(slave),
						Http::Response(statusCode, status, version) {}
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
					if (this->parseRequest(req)) {
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
		virtual void onHeaderOverflow(SlaveType *slave) {
			Response(slave, 413, "Header overflowed", "1.1").send();
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

				// Receive the next set of bytes from the slave.
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
		Body::Generator getRequestBodyGenerator(Request &req,
			std::size_t contentLength) {
			if (contentLength == 0) {
				return [](char **) { return 0; };
			}
			// Throws exception on error.
			return [this, &req, contentLength](char **bytes) {
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

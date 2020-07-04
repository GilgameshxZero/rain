#pragma once

#include "./buffer-pool.hpp"
#include "./socket.hpp"
#include "./thread-pool.hpp"

#include <functional>
#include <list>
#include <map>

#include <iostream>

namespace Rain {
	// Forward declaration.
	class HttpSocket;

	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class HttpPayload {
		public:
		std::string version;
		std::map<std::string, std::string> headers;
		char *body;
		size_t bodyReadyLen;

		HttpPayload() : body(NULL) {}
	};
	class HttpRequest : public HttpPayload {
		public:
		typedef std::function<void(HttpRequest *)> Handler;

		HttpSocket *slave;
		std::string method;
		std::string uri;

		HttpRequest(HttpSocket *slave) : HttpPayload(), slave(slave) {}
	};
	class HttpResponse : public HttpPayload {
		public:
		typedef std::function<void(HttpResponse *)> Handler;

		HttpSocket *client;
		int statusCode;
		std::string status;

		HttpResponse(HttpSocket *client) : HttpPayload(), client(client) {}
	};

	// Base class for HttpClient and HttpServerSlave.
	class HttpSocket : public Socket {
		public:
		HttpSocket(Socket socket) : Socket(socket) {}

		// Send either a request or a response.
		int send(HttpRequest req) { return 0; }
		int send(HttpResponse res) { return 0; }
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

		Handler onNewSlave = [](SlaveType *) {};
		Handler onSlaveTask = [](SlaveType *) {};
		Handler onHeaderOverflow = [](SlaveType *) {};
		HttpRequest::Handler onRequest = [](HttpRequest *) {};
		Handler onSlaveClose = [](SlaveType *) {};
		Handler onDeleteSlave = [](SlaveType *) {};

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
		~CustomHttpServer() {
			if (this->allocatedThreadPool) {
				delete this->threadPool;
			}
			if (this->allocatedBufferPool) {
				delete this->bufferPool;
			}
		}

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
					this->onNewSlave(slave);

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
		// `allocated*` is set if we allocated it in the constructor.
		ThreadPool *threadPool;
		bool allocatedThreadPool;
		BufferPool *bufferPool;
		bool allocatedBufferPool;

		// Used in parsing Http.
		inline static const char *CRLF = "\r\n";
		inline static const long long partialMatchCRLF[3] = {-1, 0, 0};

		// Slave task.
		std::function<void(void *)> slaveTask = [this](void *param) {
			SlaveType *slave = reinterpret_cast<SlaveType *>(param);
			this->onSlaveTask(slave);

			size_t bufSz = this->bufferPool->getBufSz();
			char *buf = this->bufferPool->newBuf();

			// Continuously accept requests and call onRequest when we have
			// enough information.
			while (true) {
				// Construct the request
				HttpRequest req(slave);

				// Parse the entire header.
				int recvLen;
				long long candidate = 0;
				char *curRecv = buf, *curParse = buf, *startOfLine = buf;
				while (true) {
					// Is the buffer full? If so, we can't receive the rest of the header, so need to handle that.
					if (bufSz - (curParse - buf) == 0) {
						this->onHeaderOverflow(slave);
						recvLen = -1;
						break;
					}

					// Need more data.
					recvLen = slave->recv(
						reinterpret_cast<void *>(curRecv), bufSz - (curParse - buf));
					if (recvLen <= 0) {
						// Graceful or error.
						break;
					}
					curRecv += recvLen;

					while (true) {
						// Parsing the request line. Look for end-of-line.
						char *endOfLine = Algorithm::cStrSearchKMP(curParse,
							curRecv - curParse,
							this->CRLF,
							2,
							this->partialMatchCRLF,
							&candidate);

						if (endOfLine == NULL) {
							// No end of line, so wait for the next recv.
							curParse = curRecv;
							break;
						}

						// Some end of line found.
						curParse = endOfLine + 2;

						// Determine what we're parsing based on what fields are filled out
						// from the request.
						if (req.method.length() == 0) {
							// Looking for request line.
							// Method is the first token before space.
							char *firstSpace;
							for (firstSpace = startOfLine; *firstSpace != ' '; firstSpace++)
								;
							req.method.assign(startOfLine, firstSpace - startOfLine);

							// Version is the last token after a slash.
							char *lastSlash;
							for (lastSlash = endOfLine; *lastSlash != '/'; lastSlash--)
								;
							req.version.assign(lastSlash + 1, endOfLine - 1 - lastSlash);

							// URI is stuff in between.
							req.uri.assign(firstSpace + 1, lastSlash - 6 - firstSpace);

							// Mark the start of the header line.
							startOfLine = endOfLine + 2;
						} else {
							// Looking for header block.
							// Is this line empty? If so, then we're done with the header
							// block.
							if (endOfLine == startOfLine) {
								// Past this line is the body.
								req.body = endOfLine + 2;
								req.bodyReadyLen = recvLen - (endOfLine + 2 - curParse);

								// Break to request handler.
								break;
							}

							// Look for first colon.
							char *firstColon;
							for (firstColon = startOfLine; *firstColon != ':'; firstColon++)
								;
							std::pair<std::string, std::string> header;
							header.first.assign(startOfLine, firstColon - startOfLine);
							header.second.assign(firstColon + 1, endOfLine - 1 - firstColon);

							// Trim whitespace from header key and value. Make key
							// lowercase.
							strTrimWhite(&header.first);
							strTrimWhite(&header.second);
							strToLower(&header.first);

							// Add this header to list.
							req.headers.insert(header);

							// Prepare for next.
							startOfLine = endOfLine + 2;
						}
					}

					// Did we break out because done with header parsing?
					if (req.body != NULL) {
						break;
					}
				}
				if (recvLen <= 0) {
					// We broke out because something failed, either gracefully or not.
					break;
				}

				// The request header has been parsed.
				this->onRequest(&req);
				break;
			}

			this->bufferPool->deleteBuf(buf);
			this->onSlaveClose(slave);
			slave->close();
			this->onDeleteSlave(slave);
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

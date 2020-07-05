#pragma once

#include "./buffer-pool.hpp"
#include "./socket.hpp"
#include "./string.hpp"
#include "./thread-pool.hpp"

#include <functional>
#include <list>
#include <map>
#include <set>

namespace Rain {
	// Forward declaration.
	class HttpSocket;

	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class HttpPayload {
		public:
		typedef std::function<size_t(char **)> BodyGetter;
		inline static const BodyGetter NO_BODY = [](char **) { return 0; };

		std::string version;
		std::map<std::string, std::string> headers;
		HttpSocket *socket = NULL;

		// Load more body. Returns number of characters retrieved. Sets parameter to
		// point to location of the newly retrieved body (or body to send). Returns
		// 0 to signal the end of the body.
		BodyGetter getBody = HttpPayload::NO_BODY;

		// Convenience functions for setting the body. THe parameter passed in must
		// persist until the body is sent.
		void setBody(const char *body) {
			size_t bodyLen = strlen(body);
			this->headers["content-length"] = tToStr(bodyLen);

			// Need to capture by copy; references break when trying to cast to
			// pointer again.
			this->getBody = [this, body, bodyLen](char **retBody) {
				*retBody = const_cast<char *>(body);
				size_t ret = bodyLen;

				// IMPORTANT: When getBody is set, it may damage the lambda enclosure.
				// So do everything before setting this. Same goes for all other
				// lambdas.
				this->getBody = HttpPayload::NO_BODY;
				return ret;
			};
		}
		void setBody(std::string *body) {
			this->headers["content-length"] = tToStr(body->length());
			this->getBody = [this, body](char **retBody) {
				*retBody = const_cast<char *>(body->c_str());
				size_t ret = body->length();
				this->getBody = HttpPayload::NO_BODY;
				return ret;
			};
		}

		HttpPayload(std::string version = "", HttpSocket *socket = NULL)
				: version(version), socket(socket) {
			this->headers["content-length"] = "0";
		}
		HttpPayload(HttpSocket *socket) : HttpPayload("", socket) {}
	};
	class HttpRequest : public HttpPayload {
		public:
		typedef std::function<void(HttpRequest *)> Handler;

		std::string method;
		std::string uri;

		HttpRequest(HttpSocket *socket) : HttpPayload(socket) {}
		HttpRequest(std::string method = "GET",
			std::string uri = "/",
			std::string version = "1.1")
				: HttpPayload(version), method(method), uri(uri) {}
	};
	class HttpResponse : public HttpPayload {
		public:
		typedef std::function<void(HttpResponse *)> Handler;

		std::string statusCode;
		std::string status;

		HttpResponse(HttpSocket *socket) : HttpPayload(socket) {}
		HttpResponse(std::string statusCode = "200",
			std::string status = "OK",
			std::string version = "1.1")
				: HttpPayload(version), statusCode(statusCode), status(status) {}
	};

	// Base class for HttpClient and HttpServerSlave.
	class HttpSocket : public Socket {
		public:
		HttpSocket(Socket socket) : Socket(socket) {}

		// Send either a request or a response.
		int send(HttpRequest *req) {
			Socket::send(req->method);
			Socket::send(" ");
			Socket::send(req->uri);
			Socket::send(" HTTP/");
			Socket::send(req->version);
			Socket::send("\r\n");
			this->sendHeaders(req);
			Socket::send("\r\n");
			this->sendBody(req);
			return 0;
		}
		int send(HttpResponse *res) {
			Socket::send("HTTP/");
			Socket::send(res->version);
			Socket::send(" ");
			Socket::send(res->statusCode);
			Socket::send(" ");
			Socket::send(res->status);
			Socket::send("\r\n");
			this->sendHeaders(res);
			Socket::send("\r\n");
			this->sendBody(res);
			return 0;
		}

		private:
		// Helper functions for send.
		int sendHeaders(HttpPayload *payload) {
			for (auto it = payload->headers.begin(); it != payload->headers.end();
					 it++) {
				Socket::send(it->first);
				Socket::send(":");
				Socket::send(it->second);
				Socket::send("\r\n");
			}
			return 0;
		}
		int sendBody(HttpPayload *payload) {
			char *body;
			size_t bodyLen = payload->getBody(&body);
			while (bodyLen != 0) {
				Socket::send(reinterpret_cast<void *>(body), bodyLen);
				bodyLen = payload->getBody(&body);
			}
			return 0;
		}
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
		// Allows accessing the SlaveType directly from the HttpServer class type.
		typedef SlaveType Slave;

		// Handlers for master/slave lifecycle events. Set these before serving.
		// By default, don't handle any events.
		// Can override by subclassing.
		typedef std::function<void(SlaveType *)> Handler;

		Handler onNewSlave = [](SlaveType *) {};
		Handler onBeginSlaveTask = [](SlaveType *) {};
		Handler onBeginParseHeader = [](SlaveType *) {};
		Handler onBeginSlaveRecv = [](SlaveType *) {};
		Handler onHeaderOverflow = [](SlaveType *slave) {
			HttpSocket *socket = reinterpret_cast<HttpSocket *>(slave);
			HttpResponse res("413", "Header overflowed", "1.1");
			socket->send(&res);
		};
		HttpRequest::Handler onRequest = [](HttpRequest *) {};
		Handler onCloseSlave = [](SlaveType *) {};
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
					this->slaves.insert(slave);

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

		// Closing the server should close all associated slaves as well.
		int close() {
			return Socket::close();
			for (auto it = this->slaves.begin(); it != this->slaves.end(); it++) {
				(*it)->close();
			}
		}

		private:
		// Used to spawn slaves.
		// `allocated*` is set if we allocated it in the constructor.
		ThreadPool *threadPool;
		bool allocatedThreadPool;
		BufferPool *bufferPool;
		bool allocatedBufferPool;

		// Keeps track of active slaves.
		std::set<SlaveType *> slaves;

		// Used in parsing Http.
		inline static const char *CRLF = "\r\n";
		inline static const long long partialMatchCRLF[3] = {-1, 0, 0};

		// Slave task.
		std::function<void(void *)> slaveTask = [this](void *param) {
			SlaveType *slave = reinterpret_cast<SlaveType *>(param);
			this->onBeginSlaveTask(slave);

			size_t bufSz = this->bufferPool->getBufSz();
			char *buf = this->bufferPool->newBuf();

			// Continuously accept requests and call onRequest when we have
			// enough information.
			while (true) {
				// Construct the request.
				this->onBeginParseHeader(slave);
				HttpRequest req(slave);

				// Parse the entire header.
				int recvLen;
				size_t bodyRecv = 0;	// Track the number of body bytes received.
				long long candidate = 0;
				char *curRecv = buf, *curParse = buf, *startOfLine = buf,
						 *endOfLine = NULL;
				while (true) {
					// Is the buffer full? If so, we can't receive the rest of the
					// header, so need to handle that.
					size_t bufferRemaining = bufSz - (curParse - buf);
					if (bufferRemaining == 0) {
						this->onHeaderOverflow(slave);
						recvLen = -1;
						break;
					}

					this->onBeginSlaveRecv(slave);
					recvLen =
						slave->recv(reinterpret_cast<void *>(curRecv), bufferRemaining);
					if (recvLen <= 0) {
						// Graceful or error.
						break;
					}
					curRecv += recvLen;

					while (true) {
						// Parsing the request line. Look for end-of-line.
						endOfLine = Algorithm::cStrSearchKMP(curParse,
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

						// Determine what we're parsing based on what fields are filled
						// out from the request.
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
								size_t contentLength = static_cast<size_t>(
									strtoll(req.headers["content-length"].c_str(), NULL, 10));
								// Past this line is the body.
								req.getBody = [&](char **body) {
									// This is the remaining body.
									*body = endOfLine + 2;
									bodyRecv = recvLen - (endOfLine + 2 - curParse);
									size_t ret = bodyRecv;

									// After we send the remaining body we retrieved from header
									// parsing, all body text after that needs more recv.
									req.getBody = [&](char **body) {
										this->onBeginSlaveRecv(slave);
										recvLen = slave->recv(reinterpret_cast<void *>(buf), bufSz);
										size_t ret = recvLen;
										bodyRecv += recvLen;

										*body = buf;

										// If the total received bytes is equal to the content
										// length, we are done.
										// TODO: Deal with transfer-encoding: chunked.
										if (bodyRecv >= contentLength) {
											req.getBody = HttpPayload::NO_BODY;
										}
										return ret;
									};
									return ret;
								};

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
					if (endOfLine == startOfLine) {
						break;
					}
				}
				if (recvLen <= 0) {
					// We broke out because something failed, either gracefully or not.
					break;
				}

				// The request header has been parsed.
				this->onRequest(&req);

				// TODO: Pass onto other handlers.
			}

			this->bufferPool->deleteBuf(buf);
			this->onCloseSlave(slave);
			slave->close();
			this->slaves.erase(slave);
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

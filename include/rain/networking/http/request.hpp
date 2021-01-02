#pragma once

#include "../../algorithm/kmp.hpp"
#include "../request-response/request.hpp"
#include "payload.hpp"

#include <functional>
#include <map>

namespace Rain::Networking::Http {
	// Forward declaration.
	class Response;

	class Request : public Payload,
									public RequestResponse::Request<Request, Response> {
		// Settings and constructor.
		public:
		bool overflowed;
		std::string method, path, query, fragment;

		Request(const std::string &method = "GET",
			const std::string &path = "/",
			const std::string &query = "",
			const std::string &fragment = "",
			const std::string &version = "1.1")
				: Payload(version),
					RequestResponse::Request<Request, Response>(),
					overflowed(false),
					method(method),
					path(path),
					query(query),
					fragment(fragment) {}

		// Superclass behavior.
		public:
		bool sendWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			try {
				socket.send(this->method);
				socket.send(" ");
				socket.send(this->path);
				socket.send(this->query);
				socket.send(this->fragment);
				socket.send(" HTTP/");
				socket.send(this->version);
				socket.send("\r\n");
				this->header.sendWith(socket);
				socket.send("\r\n");
				this->body.sendWith(socket);
				return false;
			} catch (...) {
				return true;
			}
		}
		bool recvWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			try {
				// Constants for KMP.
				static const char *CRLF = "\r\n";
				static const std::size_t PART_MATCH_CRLF[] = {
					(std::numeric_limits<std::size_t>::max)(), 0, 0};

				// Parse the entire header.
				// State of newline search.
				std::size_t kmpCand = 0;
				char *curRecv = socket.buf,	 // Where we're recv-ing into.
					*curParse = socket.buf,	 // Where we're parsing (not head of curRecv).
						*startOfLine =
							socket.buf,	 // The first character past the previous \r\n.
							*endOfLine = NULL;	// The \r of the next \r\n.
				bool parsingRequestLine = true;

				// Keep on calling recv until we're done.
				while (startOfLine != endOfLine) {
					// Is the buffer full? If so, we can't receive the rest of the
					// header, so need to handle that.
					std::size_t bufRemaining = socket.BUF_SZ - (curParse - socket.buf);
					if (bufRemaining == 0) {
						this->overflowed = true;
						return false;
					}

					// Receive the next set of bytes from the slave.
					std::size_t recvLen = socket.recv(
						curRecv, bufRemaining, Networking::Socket::RecvFlag::NONE);
					if (recvLen == 0) {	 // Graceful exit.
						return true;
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
						if (parsingRequestLine) {	 // Looking for request line.
							parsingRequestLine = false;

							// Method is the first token before space.
							char *firstSpace;
							for (firstSpace = startOfLine;
									 *firstSpace != ' ' && firstSpace < endOfLine;
									 firstSpace++)
								;
							this->method.assign(startOfLine, firstSpace - startOfLine);

							// Version is the last token after a slash.
							char *lastSlash;
							for (lastSlash = endOfLine;
									 *lastSlash != '/' && lastSlash - 5 > firstSpace;
									 lastSlash--)
								;
							this->version.assign(lastSlash + 1, endOfLine - 1 - lastSlash);

							// path, query, fragment is the stuff in between.
							char *queryQuestionMark;
							for (queryQuestionMark = firstSpace; *queryQuestionMark != '?' &&
									 queryQuestionMark < lastSlash - 5;
									 queryQuestionMark++)
								;
							this->path.assign(
								firstSpace + 1, queryQuestionMark - firstSpace - 1);
							char *fragmentPound;
							for (fragmentPound = queryQuestionMark;
									 *fragmentPound != '#' && fragmentPound < lastSlash - 5;
									 fragmentPound++)
								;
							this->query.assign(
								queryQuestionMark, fragmentPound - queryQuestionMark);
							this->fragment.assign(
								fragmentPound, lastSlash - fragmentPound - 5);
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
							this->header[std::string(String::findFirstNonWhitespaceCStrN(
								startOfLine, firstColon - startOfLine))] =
								std::string(String::findFirstNonWhitespaceCStrN(
									firstColon + 1, endOfLine - firstColon - 1));
						}

						// Mark the start of the next line.
						startOfLine = endOfLine + 2;
					}
				}

				// Setup body retrieval. The body we have right now starts at endOfLine
				// + 2 and goes until curRecv.
				this->body.appendBytes(endOfLine + 2, curRecv - endOfLine - 2);
				this->body.appendGenerator(getRequestBodyGenerator(socket,
					std::strtoull(this->header["Content-Length"].c_str(), NULL, 10) -
						(curRecv - endOfLine - 2)));

				return false;
			} catch (...) {
				return true;
			}
		}

		private:
		// When called with a clear buffer, will recv enough bytes from slave and
		// send to getBody caller.
		Body::Generator getRequestBodyGenerator(
			RequestResponse::Socket<Request, Response> &socket,
			std::size_t contentLength) {
			if (contentLength == 0) {
				return [](char **) { return 0; };
			}
			// Throws exception on error.
			return [this, &socket, contentLength](char **bytes) {
				std::size_t bodyLen = static_cast<std::size_t>(socket.recv(
					socket.buf, socket.BUF_SZ, Networking::Socket::RecvFlag::NONE));
				*bytes = socket.buf;
				this->body.appendGenerator(
					getRequestBodyGenerator(socket, contentLength - bodyLen));
				return bodyLen;
			};
		}
	};
}

#pragma once

#include "../../algorithm/kmp.hpp"
#include "../../string.hpp"
#include "../request-response/request-response.hpp"

#include <functional>
#include <map>

namespace Rain::Networking::Smtp {
	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class Payload {
		public:
		std::string parameter;

		Payload(const std::string &parameter = "") : parameter(parameter) {}
	};

	// Forward declaration.
	class Response;

	class Request : public RequestResponse::Request<Request, Response>,
									public Payload {
		// Settings and constructor.
		public:
		bool overflowed;
		std::string verb;

		Request(const std::string &verb = "EHLO", const std::string &parameter = "")
				: Payload(parameter), overflowed(false), verb(verb) {}

		// Superclass behavior.
		bool sendWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			try {
				socket.send(this->verb);
				if (!this->parameter.empty()) {
					socket.send(" ");
					socket.send(this->parameter);
				}
				socket.send("\r\n");
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
				char *curRecv = socket.buf;	 // Last search position.

				// Keep on calling recv until we find \r\n.
				while (true) {
					// Is the buffer full? If so, we can't receive the rest of the
					// header, so need to handle that.
					std::size_t bufRemaining = socket.BUF_SZ - (curRecv - socket.buf);
					if (bufRemaining == 0) {
						this->overflowed = true;
						return false;
					}

					// Receive the next set of bytes from the slave.
					std::size_t recvLen = socket.recv(curRecv,
						bufRemaining,
						Networking::Socket::RecvFlag::NONE,
						socket.RECV_TIMEOUT_MS);
					if (recvLen == 0) {	 // Graceful exit.
						return true;
					}

					// Parsing the request line. Look for end-of-line.
					char *newline = Algorithm::cStrSearchKMP(
						curRecv, recvLen, CRLF, 2, PART_MATCH_CRLF, &kmpCand);
					if (newline != NULL) {
						// Found newline; create request.
						char *space = socket.buf;
						for (; space != newline && *space != ' '; space++)
							;
						this->verb.assign(socket.buf, space - socket.buf);
						if (space + 1 < newline) {
							this->parameter.assign(space + 1, newline - space - 1);
						}
						break;
					}

					curRecv += recvLen;
				}

				return false;
			} catch (...) {
				return true;
			}
		}
	};
	class Response : public RequestResponse::Response<Request, Response>,
									 public Payload {
		public:
		std::size_t code;
		std::vector<std::vector<std::string>> extensions;

		Response(std::size_t code = 250, const std::string &parameter = "")
				: Payload(parameter), code(code) {}

		// Superclass behavior.
		bool sendWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			try {
				socket.send(std::to_string(this->code));
				if (!this->parameter.empty()) {
					socket.send(this->extensions.empty() ? " " : "-");
					socket.send(this->parameter);
				}
				socket.send("\r\n");
				for (std::size_t a = 0; a < this->extensions.size(); a++) {
					socket.send(std::to_string(this->code));
					socket.send(a == this->extensions.size() - 1 ? " " : "-");
					socket.send(this->extensions[a][0]);
					for (std::size_t b = 1; b < this->extensions[a].size(); b++) {
						socket.send(" ");
						socket.send(this->extensions[a][b]);
					}
					socket.send("\r\n");
				}
				return false;
			} catch (...) {
				return true;
			}
		}
		bool recvWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			try {
				static const char *CRLF = "\r\n";
				static const std::size_t PART_MATCH_CRLF[] = {
					(std::numeric_limits<std::size_t>::max)(), 0, 0};

				// Parse the entire header.
				// State of newline search.
				std::size_t kmpCand = 0;
				char *curRecv = socket.buf,	 // Last search position.
					*curParse = socket.buf, *newline;
				bool isLastLine;

				// Keep on calling recv until we find get the full response.
				this->code = 0;
				while (true) {
					std::size_t bufRemaining = socket.BUF_SZ - (curRecv - socket.buf);
					if (bufRemaining == 0) {
						return true;
					}
					std::size_t recvLen = socket.recv(curRecv,
						bufRemaining,
						Networking::Socket::RecvFlag::NONE,
						socket.RECV_TIMEOUT_MS);
					if (recvLen == 0) {
						return true;
					}

					// Look to parse an additional line.
					while (
						(newline = Algorithm::cStrSearchKMP(
							 curRecv, recvLen, CRLF, 2, PART_MATCH_CRLF, &kmpCand)) != NULL) {
						// Found full line since curParse.
						isLastLine = curParse[3] == ' ';
						curParse[3] = '\0';
						*newline = '\0';
						if (this->code == 0) {
							// First line.
							this->code = std::strtoll(curParse, NULL, 10);
							this->parameter = std::string(curParse + 4);
						} else {
							// Extension line.
							this->extensions.emplace_back(std::vector<std::string>());
							for (char *space = curParse + 4, *prevSpace = curParse + 4;
									 space != newline + 1;
									 space++) {
								if (*space == ' ' || *space == '\0') {
									*space = '\0';
									this->extensions.back().emplace_back(std::string(prevSpace));
									prevSpace = space + 1;
								}
							}
						}
						curParse = newline + 2;
						if (isLastLine) {
							break;
						}
					}
					curRecv += recvLen;
					if (isLastLine) {
						break;
					}
				}

				return false;
			} catch (...) {
				return true;
			}
		}
	};
}

#pragma once

#include "../../algorithm/kmp.hpp"
#include "../request-response/response.hpp"
#include "payload.hpp"

namespace Rain::Networking::Smtp {
	// Forward declaration.
	class Request;

	class Response : public Payload,
									 public RequestResponse::Response<Request, Response> {
		public:
		std::size_t code;
		std::vector<std::vector<std::string>> extensions;

		Response(std::size_t code = 250, std::string const &parameter = "")
				: Payload(parameter),
					RequestResponse::Response<Request, Response>(),
					code(code) {}

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
				static char const *CRLF = "\r\n";
				static std::size_t const PART_MATCH_CRLF[] = {
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
						Networking::Socket::RecvFlag::NONE);
					if (recvLen == 0) {
						return true;
					}

					// Look to parse an additional line.
					while (
						(newline = Algorithm::cStrSearchKmp(
							 curRecv, recvLen, CRLF, 2, PART_MATCH_CRLF, &kmpCand)) != NULL) {
						// Found full line since curParse.
						isLastLine = curParse[3] == ' ';
						curParse[3] = '\0';
						*newline = '\0';
						if (this->code == 0) {
							// First line.
							this->code = static_cast<std::size_t>(std::strtoll(curParse, NULL, 10));
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

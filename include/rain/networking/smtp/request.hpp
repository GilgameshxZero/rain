#pragma once

#include "../../algorithm/kmp.hpp"
#include "../request-response/request.hpp"
#include "payload.hpp"

namespace Rain::Networking::Smtp {
	// Forward declaration.
	class Response;

	class Request : public Payload,
									public RequestResponse::Request<Request, Response> {
		// Settings and constructor.
		public:
		bool overflowed;
		std::string verb;

		Request(std::string const &verb = "EHLO", std::string const &parameter = "")
				: Payload(parameter),
					RequestResponse::Request<Request, Response>(),
					overflowed(false),
					verb(verb) {}

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
				static char const *CRLF = "\r\n";
				static std::size_t const PART_MATCH_CRLF[] = {
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
						Networking::Socket::RecvFlag::NONE);
					if (recvLen == 0) {	 // Graceful exit.
						return true;
					}

					// Parsing the request line. Look for end-of-line.
					char *newline = Algorithm::cStrSearchKmp(
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
}

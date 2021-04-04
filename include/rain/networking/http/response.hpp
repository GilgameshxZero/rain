#pragma once

#include "../../algorithm/kmp.hpp"
#include "../request-response/response.hpp"
#include "payload.hpp"

namespace Rain::Networking::Http {
	// Forward declaration.
	class Request;

	class Response : public Payload,
									 public RequestResponse::Response<Request, Response> {
		public:
		std::size_t statusCode;
		std::string status;

		Response(std::size_t statusCode = 200,
			std::string const &status = "OK",
			std::string const &version = "1.1")
				: Payload(version),
					RequestResponse::Response<Request, Response>(),
					statusCode(statusCode),
					status(status) {}

		// Superclass behavior.
		bool sendWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			this->checkSetContentLength();

			if (socket.send("HTTP/") || socket.send(this->version) ||
				socket.send(" ") || socket.send(std::to_string(this->statusCode)) ||
				socket.send(" ") || socket.send(this->status) || socket.send("\r\n") ||
				this->header.sendWith(socket) || socket.send("\r\n") ||
				this->body.sendWith(socket)) {
				return true;
			}
			return false;
		}
		bool recvWith(
			RequestResponse::Socket<Request, Response> &socket) noexcept override {
			// TODO.
			return true;
		}
	};
}

#pragma once

#include "../socket.hpp"
#include "payload-request-response.hpp"

namespace Rain::Networking::Http {
	// Base class for HttpClient and HttpServerSlave.
	class Socket : virtual public Networking::Socket {
		public:
		Socket() : Networking::Socket() {}
		Socket(Networking::Socket &socket)
				: Networking::Socket(std::move(socket)) {}

		// Send either a request or a response.
		// This will modify body, so it cannot be passed as const reference.
		void send(Request *req) const {
			Networking::Socket::send(req->method);
			Networking::Socket::send(" ");
			Networking::Socket::send(req->path);
			Networking::Socket::send(req->query);
			Networking::Socket::send(req->fragment);
			Networking::Socket::send(" HTTP/");
			Networking::Socket::send(req->version);
			Networking::Socket::send("\r\n");
			this->sendHeader(&req->header);
			Networking::Socket::send("\r\n");
			this->sendBody(&req->body);
		}
		void send(Response *res) const {
			Networking::Socket::send("HTTP/");
			Networking::Socket::send(res->version);
			Networking::Socket::send(" ");
			Networking::Socket::send(std::to_string(res->statusCode));
			Networking::Socket::send(" ");
			Networking::Socket::send(res->status);
			Networking::Socket::send("\r\n");
			this->sendHeader(&res->header);
			Networking::Socket::send("\r\n");
			this->sendBody(&res->body);
		}

		private:
		// Helper functions for send.
		void sendHeader(Header *header) const {
			for (auto it = header->begin(); it != header->end(); it++) {
				Networking::Socket::send(it->first);
				Networking::Socket::send(":");
				Networking::Socket::send(it->second);
				Networking::Socket::send("\r\n");
			}
		}
		void sendBody(Body *body) const {
			char *bytes;
			std::size_t bytesLen = body->extractBytes(&bytes);
			while (bytesLen != 0) {
				Networking::Socket::send(bytes, bytesLen);
				bytesLen = body->extractBytes(&bytes);
			}
		}
	};
}

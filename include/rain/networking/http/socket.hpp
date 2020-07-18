#pragma once

#include "../socket.hpp"
#include "request-response.hpp"

namespace Rain::Networking::Http {
	// Base class for HttpClient and HttpServerSlave.
	class Socket : public Networking::Socket {
		public:
		Socket() : Networking::Socket() {}
		Socket(const Networking::Socket &socket) : Networking::Socket(socket) {}

		// Send either a request or a response.
		void send(Request *req) const {
			Networking::Socket::send(req->method);
			Networking::Socket::send(" ");
			Networking::Socket::send(req->uri);
			Networking::Socket::send(" HTTP/");
			Networking::Socket::send(req->version);
			Networking::Socket::send("\r\n");
			this->sendHeader(req);
			Networking::Socket::send("\r\n");
			this->sendBody(req);
		}
		void send(Response *res) const {
			Networking::Socket::send("HTTP/");
			Networking::Socket::send(res->version);
			Networking::Socket::send(" ");
			Networking::Socket::send(std::to_string(res->statusCode));
			Networking::Socket::send(" ");
			Networking::Socket::send(res->status);
			Networking::Socket::send("\r\n");
			this->sendHeader(res);
			Networking::Socket::send("\r\n");
			this->sendBody(res);
		}

		private:
		// Helper functions for send.
		void sendHeader(Payload *payload) const {
			// If we have bytes in the body but we don't have a Content-Length, add it
			// automatically.
			if (payload->body.getBytesLength() > 0 &&
				payload->header.find("Content-Length") == payload->header.end()) {
				payload->header["Content-Length"] =
					std::to_string(payload->body.getBytesLength());
			}
			for (auto it = payload->header.begin(); it != payload->header.end();
					 it++) {
				Networking::Socket::send(it->first);
				Networking::Socket::send(":");
				Networking::Socket::send(it->second);
				Networking::Socket::send("\r\n");
			}
		}
		void sendBody(Payload *payload) const {
			char *bytes;
			std::size_t bytesLen = payload->body.extractBytes(&bytes);
			while (bytesLen != 0) {
				Networking::Socket::send(bytes, bytesLen);
				bytesLen = payload->body.extractBytes(&bytes);
			}
		}
	};
}

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
		int send(const Request &req) const noexcept {
			Networking::Socket::send(req.method);
			Networking::Socket::send(" ");
			Networking::Socket::send(req.uri);
			Networking::Socket::send(" HTTP/");
			Networking::Socket::send(req.version);
			Networking::Socket::send("\r\n");
			this->sendHeaders(req);
			Networking::Socket::send("\r\n");
			this->sendBody(req);
			return 0;
		}
		int send(const Response &res) const noexcept {
			Networking::Socket::send("HTTP/");
			Networking::Socket::send(res.version);
			Networking::Socket::send(" ");
			Networking::Socket::send(std::to_string(res.statusCode));
			Networking::Socket::send(" ");
			Networking::Socket::send(res.status);
			Networking::Socket::send("\r\n");
			this->sendHeaders(res);
			Networking::Socket::send("\r\n");
			this->sendBody(res);
			return 0;
		}

		private:
		// Helper functions for send.
		int sendHeaders(const Payload &payload) const noexcept {
			for (auto it = payload.headers.begin(); it != payload.headers.end();
					 it++) {
				Networking::Socket::send(it->first);
				Networking::Socket::send(":");
				Networking::Socket::send(it->second);
				Networking::Socket::send("\r\n");
			}
			return 0;
		}
		int sendBody(const Payload &payload) const noexcept {
			char *body;
			size_t bodyLen = payload.getBody(&body);
			while (bodyLen != 0) {
				Networking::Socket::send(reinterpret_cast<void *>(body), bodyLen);
				bodyLen = payload.getBody(&body);
			}
			return 0;
		}
	};
}

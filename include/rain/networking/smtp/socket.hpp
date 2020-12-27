// Common utilities for SMTP server and client sockets.
#pragma once

#include "../socket.hpp"
#include "payload-request-response.hpp"

namespace Rain::Networking::Smtp {
	class Socket : virtual public Networking::Socket {
		public:
		Socket() : Networking::Socket() {}
		Socket(Networking::Socket &socket)
				: Networking::Socket(std::move(socket)) {}

		// Send either a request or a response.
		void send(Request *req) const {
			Networking::Socket::send(req->verb);
			if (!req->parameter.empty()) {
				Networking::Socket::send(" ");
				Networking::Socket::send(req->parameter);
			}
			Networking::Socket::send("\r\n");
		}
		void send(Response *res) const {
			Networking::Socket::send(std::to_string(res->code));
			if (!res->parameter.empty()) {
				Networking::Socket::send(res->extensions.empty() ? " " : "-");
				Networking::Socket::send(res->parameter);
			}
			Networking::Socket::send("\r\n");
			for (std::size_t a = 0; a < res->extensions.size(); a++) {
				Networking::Socket::send(std::to_string(res->code));
				Networking::Socket::send(a == res->extensions.size() - 1 ? " " : "-");
				Networking::Socket::send(res->extensions[a][0]);
				for (std::size_t b = 1; b < res->extensions[a].size(); b++) {
					Networking::Socket::send(" ");
					Networking::Socket::send(res->extensions[a][b]);
				}
				Networking::Socket::send("\r\n");
			}
		}

		// Ambiguity resolution via exposure.
		using Networking::Socket::send;
	};
}

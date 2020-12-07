// Common utilities for SMTP server and client sockets.
#pragma once

#include "../socket.hpp"
#include "payload-request-response.hpp"

namespace Rain::Networking::Smtp {
	class Socket : public Networking::Socket {
		public:
		Socket(Family family = Socket::Family::IPV4,
			Type type = Socket::Type::STREAM,
			Protocol protocol = Socket::Protocol::TCP)
				: Networking::Socket(family, type, protocol) {}
		Socket(const Networking::Socket &socket) : Networking::Socket(socket) {}

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
		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return Networking::Socket::send(msg, len, flags, timeoutMs);
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return Networking::Socket::send(s, flags, timeoutMs);
		}
	};
}

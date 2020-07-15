#pragma once

#include "custom-server.hpp"

namespace Rain::Networking {
	// Usable non-templated server & server slave.
	class ServerSlave
			: public CustomServerSlave<CustomServer<ServerSlave>, ServerSlave> {
		public:
		ServerSlave(const Socket &socket, CustomServer<ServerSlave> *server)
				: CustomServerSlave(socket, server) {}

		int send(const void *msg, std::size_t len, int flags = 0) const noexcept {
			return CustomServerSlave::send(msg, len, flags);
		}
		int send(const char *msg, int flags = 0) const noexcept {
			return CustomServerSlave::send(msg, flags);
		}
		int send(const std::string &s, int flags = 0) const noexcept {
			return CustomServerSlave::send(s, flags);
		}
		int recv(void *buf, std::size_t len, int flags = 0) const noexcept {
			return CustomServerSlave::recv(buf, len, flags);
		}
		int close() const noexcept { return CustomServerSlave::close(); }
	};
	typedef CustomServer<ServerSlave> Server;
}

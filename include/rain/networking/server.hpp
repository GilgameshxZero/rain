#pragma once

#include "custom-server.hpp"

namespace Rain::Networking {
	// Usable non-templated server & server slave.
	class ServerSlave
			: public CustomServerSlave<CustomServer<ServerSlave>, ServerSlave> {
		public:
		ServerSlave(const Socket &socket, CustomServer<ServerSlave> *server)
				: CustomServerSlave(socket, server) {}

		void send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL) const {
			CustomServerSlave::send(msg, len, flags);
		}
		void send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL) const {
			CustomServerSlave::send(s, flags);
		}
		int recv(char *buf, std::size_t len, int flags = 0) const {
			return CustomServerSlave::recv(buf, len, flags);
		}
		void close() { CustomServerSlave::close(); }
	};
	typedef CustomServer<ServerSlave> Server;
}

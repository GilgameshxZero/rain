#pragma once

#include "custom-server.hpp"

namespace Rain::Networking {
	// Usable non-templated server & server slave.
	class ServerSlave
			: public CustomServerSlave<CustomServer<ServerSlave>, ServerSlave, void *> {
		public:
		ServerSlave(const Socket &socket, CustomServer<ServerSlave> *server)
				: CustomServerSlave(socket, server) {}

		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(msg, len, flags, timeoutMs);
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(s, flags, timeoutMs);
		}
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::recv(buf, len, flags, timeoutMs);
		}
		void shutdown() { CustomServerSlave::shutdown(); }
	};
	typedef CustomServer<ServerSlave> Server;
}

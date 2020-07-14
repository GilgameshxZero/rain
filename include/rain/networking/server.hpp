#pragma once

#include "custom-server.hpp"

namespace Rain::Networking {
	// Usable non-templated server & server slave.
	class ServerSlave
			: public CustomServerSlave<CustomServer, ServerSlave> {
		public:
		ServerSlave(const Socket &socket, CustomServer<ServerSlave> *server)
				: CustomServerSlave(socket, server) {}
	};
	typedef CustomServer<ServerSlave> Server;
}

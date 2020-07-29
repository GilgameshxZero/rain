// Common utilities for SMTP server and client sockets.
#pragma once

#include "../socket.hpp"

namespace Rain::Networking::Smtp {
	class Socket : public Networking::Socket {
		public:
		Socket() : Networking::Socket() {}
		Socket(const Networking::Socket &socket) : Networking::Socket(socket) {}
	};
}

// SMTP client implementation.
#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	class Client : protected Smtp::Socket {};
}

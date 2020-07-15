// TODO: HTTP client.
#pragma once

#include "socket.hpp"

namespace Rain::Networking::Http {
	class Client : protected Http::Socket {};
}

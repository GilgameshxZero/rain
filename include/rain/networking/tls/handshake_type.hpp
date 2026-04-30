#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	enum class HandshakeType : std::uint8_t {
		HELLO_REQUEST = 0,
		CLIENT_HELLO,
		SERVER_HELLO
	};
};

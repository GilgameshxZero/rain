#pragma once

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	enum class ContentType : std::uint8_t {
		CHANGE_CIPHER_SPEC = 20,
		ALERT,
		HANDSHAKE,
		APPLICATION_DATA
	};
}

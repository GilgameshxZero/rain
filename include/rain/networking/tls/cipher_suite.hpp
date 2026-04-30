#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	enum class CipherSuite : std::uint16_t {
		TLS_RSA_WITH_AES_128_CBC_SHA = 0x002f
	};
}

#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	enum class CipherSuite : std::uint16_t {
		TLS_RSA_WITH_AES_128_CBC_SHA = 0x002f,
		TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA = 0xc009,
		TLS_ECCPWD_WITH_AES_128_GCM_SHA256 = 0x30b0,
		TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 = 0xcca8,
	};
}

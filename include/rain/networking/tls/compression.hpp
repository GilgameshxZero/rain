#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	enum class CompressionMethod : std::uint8_t {
		NONE = 0,
	};

	// TODO: Functions to encode/decode a buffer based on
	// compression method.
}

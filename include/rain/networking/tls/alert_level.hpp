#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	enum class AlertLevel : std::uint8_t {
		WARNING = 1,
		FATAL
	};
}

#pragma once

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class ExtensionType {
		public:
		enum Value : std::uint16_t {
			SERVER_NAME = 0x0000,
			STATUS_REQUEST = 0x0005,
			SUPPORTED_GROUPS = 0x000a,
			EC_POINT_FORMATS = 0x000b,
			SIGNATURE_ALGORITHMS = 0x000d,
			RENEGOTIATION_INFO = 0xff01,
		};

		Value value;

		ExtensionType(Value value) : value{value} {}
		operator Value() const noexcept { return this->value; }
	};
}

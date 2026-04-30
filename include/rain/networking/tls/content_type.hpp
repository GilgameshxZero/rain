#pragma once

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class ContentType {
		public:
		enum Value : std::uint8_t {
			CHANGE_CIPHER_SPEC = 20,
			ALERT,
			HANDSHAKE,
			APPLICATION_DATA
		};

		Value value;

		ContentType(Value value) : value{value} {}
		operator Value() const noexcept { return this->value; }
	};
}

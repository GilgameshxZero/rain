#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	class HandshakeType {
		public:
		enum Value : std::uint8_t {
			HELLO_REQUEST = 0,
			CLIENT_HELLO,
			SERVER_HELLO,
			SERVER_KEY_EXCHANGE = 12,
		};

		Value value;

		HandshakeType(Value value) : value{value} {}
		operator Value() const noexcept { return this->value; }
	};
};

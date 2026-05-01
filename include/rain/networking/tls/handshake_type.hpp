#pragma once

#include <cstdint>

namespace Rain::Networking::Tls {
	class HandshakeType {
		public:
		enum Value : std::uint8_t {
			HELLO_REQUEST = 0,
			CLIENT_HELLO,
			SERVER_HELLO,
			CERTIFICATE = 11,
			SERVER_KEY_EXCHANGE,
		};

		Value value;

		HandshakeType(Value value) : value{value} {}
		operator Value() const noexcept { return this->value; }
	};
};

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
			APPLICATION_DATA,
		};

		Value value;

		constexpr ContentType(Value value) : value{value} {}
		constexpr operator Value() const noexcept {
			return this->value;
		}
	};

	// A content is a full
	// Alert/Handshake/ChangeCipherSpec/ApplicationData
	// message.
	class ContentInterface {
		public:
		// TODO: bodyType.
		virtual ContentType constexpr type() const = 0;
		// Body length is length without the 4-byte header.
		virtual std::uint32_t constexpr bodyLength() const = 0;
		// Length is always the number of bytes from sendWith.
		virtual std::uint32_t constexpr length() const = 0;
		virtual void sendWith(std::ostream &) const = 0;

		virtual ~ContentInterface() {}
	};
}

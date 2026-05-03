#pragma once

#include <array>
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

		// To/from index. No better way to accomplish enum
		// reflection.
		constexpr std::size_t asIndex() const {
			switch (this->value) {
				case CHANGE_CIPHER_SPEC:
					return 0;
				case ALERT:
					return 1;
				case HANDSHAKE:
					return 2;
				case APPLICATION_DATA:
				default:
					return 3;
			}
		}
		static inline constexpr std::size_t _SIZE{4};
		static inline constexpr std::
			array<Value, ContentType::_SIZE>
				_FROM_INDEX{
					{CHANGE_CIPHER_SPEC,
						ALERT,
						HANDSHAKE,
						APPLICATION_DATA}};
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

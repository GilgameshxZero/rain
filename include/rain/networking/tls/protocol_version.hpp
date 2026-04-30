#pragma once

#include "../../algorithm/bit_manipulators.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class ProtocolVersion {
		public:
		std::uint8_t major, minor;

		// Unscoped enum allows for direct access to constants
		// from outside.
		//
		// TODO: Is there a better way to declare constants of a
		// class type, scoped to that class? Same issue with
		// *::StatusCode.
		enum Value : std::uint16_t {
			_1_0 = 0x0301,
			_1_2 = 0x0303
		};

		// Can construct from unscoped enum underlying, or
		// directly.
		ProtocolVersion(Value value) :
			major{static_cast<std::uint8_t>(value >> 8)},
			minor{static_cast<std::uint8_t>(value)} {}
		ProtocolVersion(
			std::uint8_t major,
			std::uint8_t minor) :
			major{major},
			minor{minor} {}
		ProtocolVersion(std::istream &stream) :
			major{Algorithm::readBytes<std::uint8_t>(
				stream,
				std::endian::big)},
			minor{Algorithm::readBytes<std::uint8_t>(
				stream,
				std::endian::big)} {}

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream, this->major, std::endian::big);
			Algorithm::writeBytes(
				stream, this->minor, std::endian::big);
		}
	};
}

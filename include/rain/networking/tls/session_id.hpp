#pragma once

#include "../../algorithm/bit_manipulators.hpp"

#include <climits>
#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class SessionId {
		public:
		std::vector<std::uint8_t> bytes;

		SessionId(std::vector<std::uint8_t> const &bytes) :
			bytes{bytes} {}
		SessionId(std::istream &stream) :
			bytes(
				Algorithm::readBytes<std::uint8_t>(
					stream,
					std::endian::big)) {
			stream.read(
				reinterpret_cast<char *>(this->bytes.data()),
				this->bytes.size());
		}

		std::uint8_t length() const {
			return static_cast<std::uint8_t>(this->bytes.size());
		}
		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint8_t>(this->bytes.size()),
				std::endian::big);
			stream.write(
				reinterpret_cast<char const *>(this->bytes.data()),
				this->bytes.size());
		}
	};
}

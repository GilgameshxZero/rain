#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "handshake_type.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	template<typename Body>
	class Handshake {
		public:
		Body body;

		std::uint16_t length() const {
			return 4 + body.length();
		};

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream,
				this->body.handshakeType(),
				std::endian::big);
			// Length is uint24_t.
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint32_t>(body.length()),
				std::endian::big,
				3);
			body.sendWith(stream);
		}
		void recvWith(std::istream &) const {}
	};
};

#pragma once

#include "../../algorithm/bit_manipulators.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class Random {
		public:
		std::uint8_t randomBytes[32];

		void sendWith(std::ostream &stream) const {
			stream.write(
				reinterpret_cast<char const *>(this->randomBytes),
				sizeof(randomBytes));
		}
		void recvWith(std::istream &) const {}
	};
}

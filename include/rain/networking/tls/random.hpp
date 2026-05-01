#pragma once

#include "../../algorithm/algorithm.hpp"
#include "../../algorithm/bit_manipulators.hpp"

#include <climits>
#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class Random {
		public:
		std::array<std::uint8_t, 32> randomBytes;

		Random(
			std::array<std::uint8_t, 32> const &randomBytes) :
			randomBytes{randomBytes} {}
		Random(std::istream &stream) :
			randomBytes{
				Algorithm::readBytes<std::array<std::uint8_t, 32>>(
					stream)} {}

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(stream, this->randomBytes);
		}
	};
}

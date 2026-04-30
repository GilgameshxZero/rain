#pragma once

#include "../../algorithm/bit_manipulators.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class Extension {
		public:
		// TODO.
		void sendWith(std::ostream &) const {}
		void recvWith(std::istream &) const {}
	};
}

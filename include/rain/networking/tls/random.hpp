#pragma once

#include "../tcp/socket.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class Random {
		public:
		std::uint8_t randomBytes[32];

		void sendWith(
			Tcp::ConnectedSocketSpecInterface &socket) const {
			socket.write(
				reinterpret_cast<char const *>(this->randomBytes),
				sizeof(randomBytes));
		}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
}

#pragma once

#include "../tcp/socket.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class ProtocolVersion {
		public:
		std::uint8_t major, minor;

		void sendWith(
			Tcp::ConnectedSocketSpecInterface &socket) const {
			socket.write(
				reinterpret_cast<char const *>(&this->major),
				sizeof(this->major));
			socket.write(
				reinterpret_cast<char const *>(&this->minor),
				sizeof(this->major));
		}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
}

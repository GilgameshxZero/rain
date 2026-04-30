#pragma once

#include "../tcp/socket.hpp"
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

		void sendWith(
			Tcp::ConnectedSocketSpecInterface &socket) const {
			auto msgType{this->body.handshakeType()};
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&msgType),
				sizeof(msgType));
			// Length is uint24_t.
			std::uint32_t length{body.length()};
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&length), 3);
			body.sendWith(socket);
		}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
};

// Implements TLSPlaintext from RFC 5246.
#pragma once

#include "../tcp/socket.hpp"
#include "protocol_version.hpp"
#include "tls_plaintext_content_type.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	template<typename Fragment>
	class TlsPlaintext {
		public:
		TlsPlaintextContentType type;
		ProtocolVersion version;
		Fragment fragment;

		std::uint16_t length() const {
			return fragment.length();
		};

		void sendWith(
			Tcp::ConnectedSocketSpecInterface &socket) const {
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&this->type),
				sizeof(this->type));
			this->version.sendWith(socket);
			auto length{this->length()};
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&length),
				sizeof(length));
			fragment.sendWith(socket);
		}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
}

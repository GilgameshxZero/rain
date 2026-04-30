// Implements ClientHello from RFC 5246.
#pragma once

#include "../tcp/socket.hpp"
#include "cipher_suite.hpp"
#include "extension.hpp"
#include "handshake_type.hpp"
#include "protocol_version.hpp"
#include "random.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace Rain::Networking::Tls {
	class ClientHello {
		public:
		ProtocolVersion clientVersion;
		Random random;
		std::uint8_t sessionId;
		std::vector<CipherSuite> cipherSuites;
		std::vector<std::uint8_t> compressionMethods;
		std::vector<Extension> extensions;

		HandshakeType handshakeType() const {
			return HandshakeType::CLIENT_HELLO;
		}
		std::uint16_t length() const {
			std::uint16_t extensionsLength{0};
			// TODO: Compute extensions length.
			return sizeof(this->clientVersion) +
				sizeof(this->random) + sizeof(this->sessionId) + 2 +
				cipherSuites.size() * sizeof(CipherSuite) + 1 +
				compressionMethods.size() + extensionsLength;
		}

		void sendWith(
			Tcp::ConnectedSocketSpecInterface &socket) const {
			this->clientVersion.sendWith(socket);
			this->random.sendWith(socket);
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&this->sessionId),
				sizeof(this->sessionId));
			std::uint16_t cipherSuitesSize(
				this->cipherSuites.size() * 2);
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(&cipherSuitesSize),
				sizeof(cipherSuitesSize));
			for (auto &i : this->cipherSuites) {
				socket.writeReverseEndian(
					reinterpret_cast<char const *>(&i), sizeof(i));
			}
			std::uint8_t compressionMethodsSize(
				this->compressionMethods.size());
			socket.writeReverseEndian(
				reinterpret_cast<char const *>(
					&compressionMethodsSize),
				sizeof(compressionMethodsSize));
			for (auto &i : this->compressionMethods) {
				socket.writeReverseEndian(
					reinterpret_cast<char const *>(&i), sizeof(i));
			}
			if (!this->extensions.empty()) {
				std::uint16_t extensionsSize(
					this->extensions.size());
				socket.writeReverseEndian(
					reinterpret_cast<char const *>(&extensionsSize),
					sizeof(extensionsSize));
				for (auto &i : this->extensions) {
					i.sendWith(socket);
				}
			}
		}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
}

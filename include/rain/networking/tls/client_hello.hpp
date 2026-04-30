// Implements ClientHello from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
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
			return static_cast<std::uint16_t>(
				sizeof(this->clientVersion) + sizeof(this->random) +
				sizeof(this->sessionId) + 2 +
				cipherSuites.size() * sizeof(CipherSuite) + 1 +
				compressionMethods.size() + extensionsLength);
		}

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream, this->clientVersion, std::endian::big);
			this->random.sendWith(stream);
			Algorithm::writeBytes(
				stream, this->sessionId, std::endian::big);
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint16_t>(
					this->cipherSuites.size() * 2),
				std::endian::big);
			for (auto &i : this->cipherSuites) {
				Algorithm::writeBytes(stream, i, std::endian::big);
			}
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint8_t>(
					this->compressionMethods.size()),
				std::endian::big);
			for (auto &i : this->compressionMethods) {
				Algorithm::writeBytes(stream, i, std::endian::big);
			}
			if (!this->extensions.empty()) {
				// TODO: Compute extensions length and write
				// extensions.
			}
		}
		void recvWith(std::istream &) const {}
	};
}

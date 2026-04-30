// Implements ClientHello from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "cipher_suite.hpp"
#include "extension.hpp"
#include "handshake_body.hpp"
#include "protocol_version.hpp"
#include "random.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace Rain::Networking::Tls {
	class ClientHello : public HandshakeBody {
		public:
		ProtocolVersion clientVersion;
		Random random;
		std::uint8_t sessionId;
		std::vector<CipherSuite> cipherSuites;
		std::vector<std::uint8_t> compressionMethods;
		std::vector<Extension> extensions;

		ClientHello(
			ProtocolVersion const &clientVersion,
			Random const &random,
			std::uint8_t sessionId,
			std::vector<CipherSuite> const &cipherSuites,
			std::vector<std::uint8_t> const &compressionMethods,
			std::vector<Extension> const &extensions) :
			clientVersion{clientVersion},
			random{random},
			sessionId{sessionId},
			cipherSuites{cipherSuites},
			compressionMethods{compressionMethods},
			extensions{extensions} {}
		ClientHello(std::istream &stream) :
			// Ignore the 3 length bytes.
			clientVersion{
				(stream.get(),
					stream.get(),
					stream.get(),
					ProtocolVersion(stream))},
			random(stream),
			sessionId{Algorithm::readBytes<std::uint8_t>(
				stream,
				std::endian::big)} {
			std::uint16_t cipherSuitesLength;
			Algorithm::readBytes(
				stream, cipherSuitesLength, std::endian::big);
			this->cipherSuites.resize(cipherSuitesLength / 2);
			for (auto &i : this->cipherSuites) {
				Algorithm::readBytes(stream, i, std::endian::big);
			}
			std::uint8_t compressionMethodsLength;
			Algorithm::readBytes(
				stream, compressionMethodsLength, std::endian::big);
			this->compressionMethods.resize(
				compressionMethodsLength);
			for (auto &i : this->compressionMethods) {
				Algorithm::readBytes(stream, i, std::endian::big);
			}

			// TODO: read extensions.
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::CLIENT_HELLO;
		}
		virtual std::uint32_t length() const override {
			// TODO: Compute extensions length.
			std::uint16_t extensionsLength{0};

			return static_cast<std::uint32_t>(
				sizeof(this->clientVersion) + sizeof(this->random) +
				sizeof(this->sessionId) + 2 +
				cipherSuites.size() * sizeof(CipherSuite) + 1 +
				compressionMethods.size() + extensionsLength);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
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
	};
}

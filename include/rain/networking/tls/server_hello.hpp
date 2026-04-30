// Implements ServerHello from RFC 5246.
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
	class ServerHello : public HandshakeBody {
		public:
		ProtocolVersion serverVersion;
		Random random;
		std::uint8_t sessionId;
		CipherSuite cipherSuite;
		std::uint8_t compressionMethod;
		std::vector<Extension> extensions;

		ServerHello(
			ProtocolVersion const &serverVersion,
			Random const &random,
			std::uint8_t sessionId,
			CipherSuite const &cipherSuite,
			std::uint8_t compressionMethod,
			std::vector<Extension> const &extensions) :
			serverVersion{serverVersion},
			random{random},
			sessionId{sessionId},
			cipherSuite{cipherSuite},
			compressionMethod{compressionMethod},
			extensions{extensions} {}
		ServerHello(std::istream &stream) :
			// Ignore the 3 length bytes.
			// TODO: We can't do this, because we need to
			// determine if extensions exist.
			serverVersion{
				(stream.get(),
					stream.get(),
					stream.get(),
					ProtocolVersion(stream))},
			random(stream),
			sessionId{Algorithm::readBytes<std::uint8_t>(
				stream,
				std::endian::big)} {
			Algorithm::readBytes(
				stream, this->cipherSuite, std::endian::big);
			Algorithm::readBytes(
				stream, this->compressionMethod, std::endian::big);

			// TODO: read extensions.
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::SERVER_HELLO;
		}
		virtual std::uint32_t length() const override {
			// TODO: Compute extensions length.
			std::uint16_t extensionsLength{0};

			return static_cast<std::uint32_t>(
				sizeof(this->serverVersion) + sizeof(this->random) +
				sizeof(this->sessionId) +
				sizeof(this->cipherSuite) +
				sizeof(this->compressionMethod) + extensionsLength);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream, this->serverVersion, std::endian::big);
			this->random.sendWith(stream);
			Algorithm::writeBytes(
				stream, this->sessionId, std::endian::big);
			Algorithm::writeBytes(
				stream, this->cipherSuite, std::endian::big);
			Algorithm::writeBytes(
				stream, this->compressionMethod, std::endian::big);

			if (!this->extensions.empty()) {
				// TODO: Compute extensions length and write
				// extensions.
			}
		}
	};
}

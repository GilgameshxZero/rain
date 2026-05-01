// Implements ServerHello from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "cipher_suite.hpp"
#include "handshake_body.hpp"
#include "protocol_version.hpp"
#include "random.hpp"
#include "session_id.hpp"
#include "tls_extension.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace Rain::Networking::Tls {
	class ServerHello : public HandshakeBody {
		private:
		// Only used in constructor.
		std::uint32_t completeLength;

		std::uint16_t computeExtensionsLength() const {
			if (this->extensions.empty()) {
				return 0;
			}
			std::uint16_t extensionsLength{};
			for (auto &i : this->extensions) {
				// 4 for each extension length and type.
				extensionsLength += i.extensionData->length() + 4;
			}
			return extensionsLength;
		}

		public:
		ProtocolVersion serverVersion;
		Random random;
		SessionId sessionId;
		CipherSuite cipherSuite;
		std::uint8_t compressionMethod;
		std::vector<TlsExtension> extensions;

		ServerHello(
			ProtocolVersion const &serverVersion,
			Random const &random,
			SessionId const &sessionId,
			CipherSuite const &cipherSuite,
			std::uint8_t compressionMethod,
			std::vector<TlsExtension> const &extensions) :
			completeLength{0},
			serverVersion{serverVersion},
			random{random},
			sessionId{sessionId},
			cipherSuite{cipherSuite},
			compressionMethod{compressionMethod},
			extensions{extensions} {}
		ServerHello(std::istream &stream) :
			completeLength{Algorithm::readBytes<std::uint32_t>(
				stream,
				std::endian::big,
				3)},
			serverVersion{ProtocolVersion(stream)},
			random(stream),
			sessionId(stream) {
			Algorithm::readBytes(
				stream, this->cipherSuite, std::endian::big);
			Algorithm::readBytes(
				stream, this->compressionMethod, std::endian::big);

			if (completeLength == this->length()) {
				return;
			}

			auto extensionsLength{
				Algorithm::readBytes<std::uint16_t>(
					stream, std::endian::big)};
			// If extensionsLength wraps around and this loop gets
			// stuck, that's the fault of the caller and the
			// socket will eventually time out and error.
			while (extensionsLength > 0) {
				this->extensions.emplace_back(stream);
				// 4 for the extension type and length.
				extensionsLength -= 4 +
					this->extensions.back().extensionData->length();
			}
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::SERVER_HELLO;
		}
		virtual std::uint32_t length() const override {
			std::uint16_t extensionsLength{
				this->computeExtensionsLength()};

			return static_cast<std::uint32_t>(
				sizeof(this->serverVersion) + sizeof(this->random) +
				1 + this->sessionId.length() +
				sizeof(this->cipherSuite) +
				sizeof(this->compressionMethod) +
				(extensionsLength > 0 ? 2 + extensionsLength : 0));
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream, this->serverVersion, std::endian::big);
			this->random.sendWith(stream);
			this->sessionId.sendWith(stream);
			Algorithm::writeBytes(
				stream, this->cipherSuite, std::endian::big);
			Algorithm::writeBytes(
				stream, this->compressionMethod, std::endian::big);

			if (!this->extensions.empty()) {
				Algorithm::writeBytes(
					stream,
					this->computeExtensionsLength(),
					std::endian::big);
				for (auto &i : this->extensions) {
					i.sendWith(stream);
				}
			}
		}
	};
}

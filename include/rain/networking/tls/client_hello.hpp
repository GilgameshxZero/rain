// Implements ClientHello from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "../../literal.hpp"
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
	class ClientHello : public HandshakeBody {
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
		ProtocolVersion clientVersion;
		Random random;
		SessionId sessionId;
		std::vector<CipherSuite> cipherSuites;
		std::vector<std::uint8_t> compressionMethods;
		std::vector<TlsExtension> extensions;

		ClientHello(
			ProtocolVersion const &clientVersion,
			Random const &random,
			SessionId const &sessionId,
			std::vector<CipherSuite> const &cipherSuites,
			std::vector<std::uint8_t> const &compressionMethods,
			std::vector<TlsExtension> const &extensions) :
			completeLength{0},
			clientVersion{clientVersion},
			random{random},
			sessionId{sessionId},
			cipherSuites{cipherSuites},
			compressionMethods{compressionMethods},
			extensions{extensions} {}
		ClientHello(std::istream &stream) :
			completeLength{Algorithm::readBytes<std::uint32_t>(
				stream,
				std::endian::big,
				3)},
			clientVersion{ProtocolVersion(stream)},
			random(stream),
			sessionId(stream) {
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

			if (completeLength == this->length()) {
				return;
			}

			std::int32_t extensionsLength(
				Algorithm::readBytes<std::uint16_t>(
					stream, std::endian::big));
			// If extensionsLength wraps around and this loop gets
			// stuck, that's the fault of the caller and the
			// socket will eventually time out and error.
			while (stream && extensionsLength > 0) {
				this->extensions.emplace_back(stream);
				// 4 for the extension type and length.
				extensionsLength -= 4 +
					this->extensions.back().extensionData->length();
			}
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::CLIENT_HELLO;
		}
		virtual std::uint32_t length() const override {
			std::uint16_t extensionsLength{
				this->computeExtensionsLength()};

			return static_cast<std::uint32_t>(
				sizeof(this->clientVersion) + sizeof(this->random) +
				1 + this->sessionId.length() + 2 +
				cipherSuites.size() * sizeof(CipherSuite) + 1 +
				compressionMethods.size() +
				(extensionsLength > 0 ? 2 + extensionsLength : 0));
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream, this->clientVersion, std::endian::big);
			this->random.sendWith(stream);
			this->sessionId.sendWith(stream);
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

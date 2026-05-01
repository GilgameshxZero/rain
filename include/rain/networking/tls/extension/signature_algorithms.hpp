#pragma once

#include "../../../algorithm/bit_manipulators.hpp"
#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class SignatureAlgorithms : public ExtensionData {
		public:
		enum SignatureScheme : std::uint16_t {
			RSA_PKCS1_SHA256 = 0x0401,
			ECDSA_SECP256R1_SHA256 = 0x0403,
		};

		std::vector<SignatureScheme> signatureSchemes;

		SignatureAlgorithms(
			std::vector<SignatureScheme> const
				&signatureSchemes) :
			signatureSchemes{signatureSchemes} {}
		SignatureAlgorithms(std::istream &stream) :
			signatureSchemes(
				Algorithm::readBytes<std::uint16_t>(
					stream,
					std::endian::big) /
				2) {
			for (auto &i : this->signatureSchemes) {
				Algorithm::readBytes(stream, i, std::endian::big);
			}
		}

		virtual ExtensionType extensionType() const override {
			return ExtensionType::SIGNATURE_ALGORITHMS;
		}
		virtual std::uint16_t length() const override {
			return static_cast<std::uint16_t>(
				2 + this->signatureSchemes.size() * 2);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint16_t>(
					this->signatureSchemes.size() * 2),
				std::endian::big);
			for (auto &i : this->signatureSchemes) {
				Algorithm::writeBytes(stream, i, std::endian::big);
			}
		}
	};
}

#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	// TODO: Don't hardcode.
	class SignatureAlgorithms : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::SIGNATURE_ALGORITHMS;
		}
		virtual std::uint16_t length() const override {
			return 0x0006;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			// Minimal set to support.
			stream << "\x00\x04\x04\x03\x04\x01"s;
		}
	};
}

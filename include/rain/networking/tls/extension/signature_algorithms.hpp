#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class SignatureAlgorithms : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::SIGNATURE_ALGORITHMS;
		}
		virtual std::uint16_t length() const override {
			return 0x002a;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			stream
				<< "\x00\x28\x04\x03\x05\x03\x06\x03\x08\x07\x08\x08\x08\x09\x08\x0a\x08\x0b\x08\x04\x08\x05\x08\x06\x04\x01\x05\x01\x06\x01\x03\x03\x03\x01\x03\x02\x04\x02\x05\x02\x06\x02"s;
		}
	};
}

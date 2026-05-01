#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	// TODO: Don't hardcode. Not important.
	class RenegotiationInfo : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::RENEGOTIATION_INFO;
		}
		virtual std::uint16_t length() const override {
			return 0x0001;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			stream << "\x00"s;
		}
	};
}

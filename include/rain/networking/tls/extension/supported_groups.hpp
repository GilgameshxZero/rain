#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	// TODO: Don't hardcode.
	class SupportedGroups : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::SUPPORTED_GROUPS;
		}
		virtual std::uint16_t length() const override {
			return 0x0006;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			// Minimal set, both important.
			stream << "\x00\x04\x00\x17\x00\x18"s;
		}
	};
}

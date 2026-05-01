#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class SupportedGroups : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::SUPPORTED_GROUPS;
		}
		virtual std::uint16_t length() const override {
			return 0x000c;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			stream
				<< "\x00\x0a\x00\x1d\x00\x17\x00\x1e\x00\x18\x00\x19"s;
		}
	};
}

#pragma once

#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class EcPointFormats : public ExtensionData {
		public:
		virtual ExtensionType extensionType() const override {
			return ExtensionType::EC_POINT_FORMATS;
		}
		virtual std::uint16_t length() const override {
			return 0x0002;
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			stream << "\x01\x00"s;
		}
	};
}

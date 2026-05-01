#pragma once

#include "../../../algorithm/bit_manipulators.hpp"
#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class SupportedGroups : public ExtensionData {
		public:
		enum NamedCurve : std::uint16_t {
			SECP256R1 = 0x0017,
			SECP384R1,
		};

		std::vector<NamedCurve> namedCurves;

		SupportedGroups(
			std::vector<NamedCurve> const &namedCurves) :
			namedCurves{namedCurves} {}
		SupportedGroups(std::istream &stream) :
			namedCurves(
				Algorithm::readBytes<std::uint16_t>(
					stream,
					std::endian::big) /
				2) {
			for (auto &i : this->namedCurves) {
				Algorithm::readBytes(stream, i, std::endian::big);
			}
		}

		virtual ExtensionType extensionType() const override {
			return ExtensionType::SUPPORTED_GROUPS;
		}
		virtual std::uint16_t length() const override {
			return static_cast<std::uint16_t>(
				2 + this->namedCurves.size() * 2);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint16_t>(
					this->namedCurves.size() * 2),
				std::endian::big);
			for (auto &i : this->namedCurves) {
				Algorithm::writeBytes(stream, i, std::endian::big);
			}
		}
	};
}

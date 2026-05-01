#pragma once

#include "../../../algorithm/bit_manipulators.hpp"
#include "../../../literal.hpp"
#include "../extension_data.hpp"
#include "../extension_type.hpp"

namespace Rain::Networking::Tls::Extension {
	class ServerName : public ExtensionData {
		public:
		std::vector<std::string> serverNames;

		ServerName(
			std::vector<std::string> const &serverNames) :
			serverNames{serverNames} {}
		ServerName(std::istream &stream) {
			auto serverNamesLength{
				Algorithm::readBytes<std::uint16_t>(
					stream, std::endian::big)};
			while (serverNamesLength > 0) {
				// Always type 0, DNS hostname.
				stream.get();
				auto hostnameLength{
					Algorithm::readBytes<std::uint16_t>(
						stream, std::endian::big)};
				this->serverNames.emplace_back(
					hostnameLength, '\0');
				stream.read(
					this->serverNames.back().data(), hostnameLength);
				serverNamesLength -= 3 + hostnameLength;
			}
		}

		virtual ExtensionType extensionType() const override {
			return ExtensionType::SERVER_NAME;
		}
		virtual std::uint16_t length() const override {
			if (this->serverNames.empty()) {
				return 0;
			}
			std::size_t namesLength{};
			for (auto &i : this->serverNames) {
				namesLength += i.length() + 3;
			}
			return static_cast<std::uint16_t>(namesLength + 2);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			if (this->serverNames.empty()) {
				return;
			}
			Algorithm::writeBytes(
				stream,
				static_cast<std::uint16_t>(this->length() - 2),
				std::endian::big);
			for (auto &i : this->serverNames) {
				// Type 0, DNS hostname.
				stream.put('\x00');
				Algorithm::writeBytes(
					stream,
					static_cast<std::uint16_t>(i.length()),
					std::endian::big);
				stream << i;
			}
		}
	};
}

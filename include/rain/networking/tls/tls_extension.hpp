// <https://www.iana.org/assignments/tls-extensiontype-values/tls-extensiontype-values.xhtml>.
#pragma once

// Also includes all extension types in subdirectory.
#include "../../algorithm/bit_manipulators.hpp"
#include "extension.hpp"
#include "extension_data.hpp"
#include "extension_type.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

namespace Rain::Networking::Tls {
	// Named TlsExtension instead of Extension to distinguish
	// from Extension namespace.
	class TlsExtension {
		private:
		// Not used outside of constructor.
		ExtensionType type;

		ExtensionData *makeExtensionData(
			ExtensionType const &type,
			auto &&...args) {
			switch (type) {
				case ExtensionType::SERVER_NAME:
					return new Extension::ServerName(
						std::forward<decltype(args)>(args)...);
				case ExtensionType::SIGNATURE_ALGORITHMS:
					return new Extension::SignatureAlgorithms(
						std::forward<decltype(args)>(args)...);
				case ExtensionType::SUPPORTED_GROUPS:
				default:
					return new Extension::SupportedGroups(
						std::forward<decltype(args)>(args)...);
			}
		};

		public:
		// shared_ptr to support copy constructor.
		std::shared_ptr<ExtensionData> extensionData;

		TlsExtension(ExtensionData *extensionData) :
			type{extensionData->extensionType()},
			extensionData(extensionData) {}
		TlsExtension(std::istream &stream) :
			type(
				Algorithm::readBytes<ExtensionType::Value>(
					stream,
					std::endian::big)),
			extensionData{
				this->makeExtensionData(this->type, stream)} {}

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream,
				this->extensionData->extensionType(),
				std::endian::big);
			Algorithm::writeBytes(
				stream,
				this->extensionData->length(),
				std::endian::big);
			this->extensionData->sendWith(stream);
		}
	};
}

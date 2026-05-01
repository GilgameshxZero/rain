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
			auto &&...) {
			switch (type) {
				default:
					return nullptr;
			}
		};

		public:
		// shared_ptr to support copy constructor.
		std::shared_ptr<ExtensionData> extensionData;

		TlsExtension(ExtensionData *extensionData) :
			type{extensionData->extensionType()},
			extensionData(extensionData) {}

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

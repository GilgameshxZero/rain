// Implements TLSPlaintext from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "content_type.hpp"
#include "protocol_version.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	// TODO: This interface needs to be changed. Instead of
	// templating, we probably need to dynamically allocate a
	// fragment inside. This is because during `recv`, we
	// cannot know the ContentType in advance.
	template<typename Fragment>
	class Plaintext {
		public:
		ContentType type;
		ProtocolVersion version;
		Fragment fragment;

		std::uint16_t length() const {
			return fragment.length();
		};

		void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream, this->type, std::endian::big);
			this->version.sendWith(stream);
			Algorithm::writeBytes(
				stream, this->length(), std::endian::big);
			fragment.sendWith(stream);
			stream.flush();
		}
		void recvWith(std::istream &) const {}
	};
}

// Implements TLSPlaintext from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "alert.hpp"
#include "content_type.hpp"
#include "handshake.hpp"
#include "plaintext_fragment.hpp"
#include "protocol_version.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

namespace Rain::Networking::Tls {
	// TODO: Record layer data may be fragmented or joined
	// together within one/multiple
	// Plaintext/Ciphertext/Compressed, or even interleaved.
	// So, TLS sockets should maintain a separate streambuf
	// for each content type.
	class Plaintext {
		private:
		// Not used outside of constructor.
		ContentType type;

		PlaintextFragment *makePlaintextFragment(
			ContentType const &type,
			auto &&...args) {
			switch (type) {
				case ContentType::ALERT:
					return new Alert(
						std::forward<decltype(args)>(args)...);
				case ContentType::HANDSHAKE:
				default:
					return new Handshake(
						std::forward<decltype(args)>(args)...);
			}
		}

		public:
		ProtocolVersion version;
		// shared_ptr to support copy constructor.
		std::shared_ptr<PlaintextFragment> fragment;

		// The virtual member functions disqualify us as an
		// aggergate class, so we provide a constructor instead.
		Plaintext(
			ProtocolVersion const &version,
			PlaintextFragment *fragment) :
			type{fragment->contentType()},
			version{version},
			fragment(fragment) {}
		// We do not provide a recvWith function. Instead, to
		// guarantee that Plaintext is always in a valid state
		// (outside of constructors), we provide a constructor
		// that constructs from an istream.
		Plaintext(std::istream &stream) :
			type{Algorithm::readBytes<ContentType::Value>(
				stream,
				std::endian::big)},
			version(stream),
			fragment(
				this->makePlaintextFragment(this->type, stream)) {}

		virtual void sendWith(std::ostream &stream) const {
			Algorithm::writeBytes(
				stream,
				this->fragment->contentType(),
				std::endian::big);
			this->version.sendWith(stream);
			Algorithm::writeBytes(
				stream, this->fragment->length(), std::endian::big);
			this->fragment->sendWith(stream);
			stream.flush();
		}
	};
}

#pragma once

#include "content_interface.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	enum class RecordType {
		PLAINTEXT,
		COMPRESSED,
		CIPHERTEXT
	};

	// A record is TlsPlaintext/TlsCiphertext/TlsCompressed as
	// defined in RFC 5246.
	class RecordInterface : virtual public std::iostream {
		public:
		// Records contain arbitrary data in its fragment, and
		// thus is best represented as a stringstream with an
		// internal buffer. The stringstream must error when the
		// record is finished; otherwise, if there is no error,
		// it will read from an underlying stream (commonly, the
		// TCP socket if receiving, or another source if
		// sending).
		virtual RecordType type() const = 0;
		virtual std::uint16_t fragmentLength() const = 0;
		virtual std::uint16_t length() const = 0;
		virtual void sendWith(std::ostream &) const = 0;

		virtual ~RecordInterface() {}
	};
}

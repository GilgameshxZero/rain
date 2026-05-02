// Implements TLSPlaintext from RFC 5246.
#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "alert.hpp"
#include "content_interface.hpp"
#include "handshake.hpp"
#include "protocol_version.hpp"
#include "record_interface.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>

namespace Rain::Networking::Tls {
	// class PlaintextRecord : virtual public RecordInterface {
	// 	public:
	// 	ContentType contentType;
	// 	ProtocolVersion version;

	// 	virtual ~PlaintextRecord() { delete this->rdbuf(); }

	// 	// Construct with string-like.
	// 	// Default constructor is empty body.
	// 	PlaintextRecord(
	// 		ContentType const &contentType,
	// 		ProtocolVersion const &version,
	// 		std::string const &str = "") :
	// 		contentType{contentType},
	// 		version{version},
	// 		std::iostream(
	// 			str.empty() ? nullptr : new std::stringbuf(str)) {}
	// 	PlaintextRecord(
	// 		ContentType const &contentType,
	// 		ProtocolVersion const &version,
	// 		char const *cStr) :
	// 		PlaintextRecord(
	// 			contentType,
	// 			version,
	// 			std::string(cStr)) {}
	// 	// Construct with buffer-like will move in the buffer.
	// 	PlaintextRecord(
	// 		ContentType const &contentType,
	// 		ProtocolVersion const &version,
	// 		std::stringbuf &&stringbuf) :
	// 		contentType{contentType},
	// 		version{version},
	// 		std::iostream(
	// 			new std::stringbuf(std::move(stringbuf))) {}
	// 	PlaintextRecord(
	// 		ContentType const &contentType,
	// 		ProtocolVersion const &version,
	// 		std::filebuf &&filebuf) :
	// 		contentType{contentType},
	// 		version{version},
	// 		std::iostream(new std::filebuf(std::move(filebuf))) {}

	// 	// Cannot be copied because the buffer cannot be copied.
	// 	PlaintextRecord(PlaintextRecord const &) = delete;
	// 	PlaintextRecord &operator=(
	// 		PlaintextRecord const &) = delete;

	// 	// Move constructor also swaps streambufs and freeing
	// 	// responsibility.
	// 	PlaintextRecord(PlaintextRecord &&other) :
	// 		contentType{other.contentType},
	// 		version{other.version},
	// 		std::iostream(std::move(other)) {
	// 		this->rdbuf(other.rdbuf(nullptr));
	// 	}
	// 	// Move assignment transfers memory responsibility to
	// 	// this object.
	// 	PlaintextRecord &operator=(PlaintextRecord &&other) {
	// 		std::iostream::operator=(std::move(other));
	// 		this->rdbuf(other.rdbuf(nullptr));
	// 		return *this;
	// 	}

	// 	// Hide memory-sensitive setter.
	// 	private:
	// 	std::streambuf *rdbuf(std::streambuf *sb) {
	// 		return std::iostream::rdbuf(sb);
	// 	}

	// 	public:
	// 	std::streambuf *rdbuf() const {
	// 		return std::istream::rdbuf();
	// 	}

	// 	// Helper for calling in_avail on rdbuf.
	// 	std::streamsize inAvail() const {
	// 		if (this->rdbuf() == nullptr) {
	// 			return -1;
	// 		}
	// 		return this->rdbuf()->in_avail();
	// 	}

	// 	// Stream operator. Cannot stream in >> directly, since
	// 	// need to know Content-Length/Transfer-Encoding.
	// 	friend inline std::ostream &operator<<(
	// 		std::ostream &stream,
	// 		PlaintextRecord &record) {
	// 		// Will set failbit on stream if rdbuf is empty.
	// 		stream << record.rdbuf();
	// 		stream.clear();
	// 		return stream;
	// 	}

	// 	virtual RecordType type() const override {
	// 		return RecordType::PLAINTEXT;
	// 	}
	// 	virtual std::uint16_t fragmentLength() const override {
	// 		return this->fragment->length();
	// 	}
	// 	virtual std::uint16_t length() const override {
	// 		return this->fragmentLength() + 5;
	// 	}
	// 	virtual void sendWith(
	// 		std::ostream &stream) const override {
	// 		Algorithm::writeBytes(
	// 			stream, this->contentType, std::endian::big);
	// 		this->version.sendWith(stream);
	// 		Algorithm::writeBytes(
	// 			stream, this->fragment->length(), std::endian::big);
	// 		this->fragment->sendWith(stream);
	// 		stream.flush();
	// 	}
	// };

	// TODO: Record layer data may be fragmented or joined
	// together within one/multiple Plaintext/Ciphertext, or
	// even interleaved. So, TLS sockets should maintain a
	// separate buffer for each content type.
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

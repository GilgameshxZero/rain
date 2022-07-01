// Generic message format for RFC 822 TCP-based HTTP.
// generic-message = start-line
//									*(message-header CRLF)
//									CRLF
//									[ message-body ]
// start-line      = Request-Line | Status-Line
#pragma once

#include "../../string.hpp"
#include "../req-res/message.hpp"
#include "body.hpp"
#include "headers.hpp"
#include "version.hpp"

namespace Rain::Networking::Http {
	class MessageSpecInterface : virtual public ReqRes::MessageInterface {
		public:
		enum class Error {
			MALFORMED_TRANSFER_ENCODING = 1,
			MALFORMED_CONTENT_LENGTH,
			TRANSFER_ENCODING_NOT_SUPPORTED,
			TOO_MANY_TRANSFER_ENCODINGS
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Http::Message";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::MALFORMED_TRANSFER_ENCODING:
						return "Malformed HTTP header Transfer-Encoding.";
					case Error::MALFORMED_CONTENT_LENGTH:
						return "Malformed HTTP header Content-Length";
					case Error::TRANSFER_ENCODING_NOT_SUPPORTED:
						return "Transfer-Encoding not supported.";
					case Error::TOO_MANY_TRANSFER_ENCODINGS:
						return "Too many Transfer-Encodings.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		protected:
		// streambufs which interpret a single layer of Transfer-Encoding, given the
		// right initial settings.
		class IdentityTransferEncodingIStreamBuf : public std::streambuf {
			private:
			// The TCP Socket stream from which we will read from to fill up this
			// streambuf.
			std::streambuf *const sourceStreamBuf;

			// Content length, if available. If SIZE_MAX, then we will read until
			// sourceStreamBuf is invalidated.
			std::size_t contentLengthRemaining;

			// Internal buffer.
			std::string buffer;

			public:
			IdentityTransferEncodingIStreamBuf(
				std::streambuf *sourceStreamBuf,
				std::size_t contentLength = SIZE_MAX,
				std::size_t bufferLen = 1_zu << 10)
					: sourceStreamBuf(sourceStreamBuf),
						contentLengthRemaining(contentLength) {
				this->buffer.reserve(bufferLen);
				this->buffer.resize(this->buffer.capacity());

				// Set internal pointers for empty buffers.
				this->setg(&this->buffer[0], &this->buffer[0], &this->buffer[0]);
			}

			// Disable copy.
			IdentityTransferEncodingIStreamBuf(
				IdentityTransferEncodingIStreamBuf const &) = delete;
			IdentityTransferEncodingIStreamBuf &operator=(
				IdentityTransferEncodingIStreamBuf const &) = delete;

			protected:
			// Re-fill the buffer from the source, decrementing contentLength if
			// necessary.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					std::size_t cFilled{
						static_cast<std::size_t>(this->sourceStreamBuf->sgetn(
							&this->buffer[0],
							std::min(this->buffer.length(), this->contentLengthRemaining)))};
					this->contentLengthRemaining -=
						this->contentLengthRemaining == SIZE_MAX ? 0 : cFilled;

					// None filled, so we are done (or the Socket has been closed/etc.).
					if (cFilled == 0) {
						return traits_type::eof();
					}

					this->setg(
						&this->buffer[0], &this->buffer[0], &this->buffer[0] + cFilled);
				}
				return traits_type::to_int_type(*this->gptr());
			}
		};

		class ChunkedTransferEncodingIStreamBuf : public std::streambuf {
			private:
			// The TCP Socket stream from which we will read from to fill up this
			// streambuf.
			std::streambuf *const sourceStreamBuf;

			// Bytes remaining in the current chunk. Can be zero, in which case we
			// process the next chunk.
			std::size_t chunkLenRemaining;

			// Internal buffer.
			std::string buffer;

			public:
			ChunkedTransferEncodingIStreamBuf(
				std::streambuf *sourceStreamBuf,
				std::size_t bufferLen = 1_zu << 10)
					: sourceStreamBuf(sourceStreamBuf), chunkLenRemaining(0) {
				this->buffer.reserve(bufferLen);
				this->buffer.resize(this->buffer.capacity());

				// Set internal pointers for empty buffers.
				this->setg(&this->buffer[0], &this->buffer[0], &this->buffer[0]);
			}

			// Disable copy.
			ChunkedTransferEncodingIStreamBuf(
				ChunkedTransferEncodingIStreamBuf const &) = delete;
			ChunkedTransferEncodingIStreamBuf &operator=(
				ChunkedTransferEncodingIStreamBuf const &) = delete;

			protected:
			// Re-fill the buffer from the source, decrementing contentLength if
			// necessary.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					// If chunkLenRemaining is 0, we need to interpret it more.
					if (this->chunkLenRemaining == 0) {
						std::string chunkLenBuffer(20, '\0');
						std::istream chunkLenStream(this->sourceStreamBuf);
						chunkLenStream.getline(
							&chunkLenBuffer[0], chunkLenBuffer.length(), '\n');

						// -2 for \r\0.
						chunkLenBuffer.resize(static_cast<std::size_t>(
							std::max(std::streamsize(0), chunkLenStream.gcount() - 2)));
						this->chunkLenRemaining = static_cast<std::size_t>(
							std::strtoumax(chunkLenBuffer.c_str(), NULL, 10));

						// If chunk length parsed to 0, we are done.
						if (this->chunkLenRemaining == 0) {
							return traits_type::eof();
						}
					}

					// Track chunk length.
					std::size_t cFilled{
						static_cast<std::size_t>(this->sourceStreamBuf->sgetn(
							&this->buffer[0],
							std::min(this->buffer.length(), this->chunkLenRemaining)))};
					this->chunkLenRemaining -= cFilled;

					// None filled, so the stream has probably been closed.
					if (cFilled == 0) {
						return traits_type::eof();
					}

					// If chunkLenRemaining has just been set to 0, ignore the \r\n coming
					// right after.
					if (this->chunkLenRemaining == 0) {
						this->sourceStreamBuf->sbumpc();
						this->sourceStreamBuf->sbumpc();
					}

					this->setg(
						&this->buffer[0], &this->buffer[0], &this->buffer[0] + cFilled);
				}

				return traits_type::to_int_type(*this->gptr());
			}
		};

		public:
		virtual void recvBody(std::istream &stream) = 0;
		virtual void ppEstimateContentLength(bool) = 0;
		virtual void ppDefaultContentType() = 0;
	};

	template <typename Message>
	class MessageSpec : public Message, virtual public MessageSpecInterface {
		public:
		// Version of the start line is shared b/t R/R.
		Version version;

		// Header block.
		Headers headers;

		// Body is an uncopyable ostream wrapping a custom streambuf.
		Body body;

		private:
		// Dynamic storage for streambufs created for body parsing.
		std::vector<std::unique_ptr<std::streambuf>> bodyTransferEncodingStreamBufs;

		public:
		// Constructor arguments passed in from R/R. Move construct heavy arguments
		// with rvalue references.
		MessageSpec(Headers &&headers = {}, Body &&body = {}, Version version = {})
				: version(version),
					headers(std::move(headers)),
					body(std::move(body)) {}

		// Must enable move construct for PreResponse.
		MessageSpec(MessageSpec &&other)
				: Message(std::move(other)),
					version(other.version),
					headers(std::move(other.headers)),
					body(std::move(other.body)),
					bodyTransferEncodingStreamBufs(
						std::move(other.bodyTransferEncodingStreamBufs)) {}

		// Pre/post-processors.
		public:
		// Process sets Content-Length if possible.
		virtual void ppEstimateContentLength(bool allowZero) override {
			std::vector<Header::TransferEncoding> transferEncoding =
				this->headers.transferEncoding();

			// Set Content-Length if not yet set and Transfer-Encoding is identity and
			// res.body's length can be determined.
			if (
				this->headers.find("Content-Length") == this->headers.end() &&
				(transferEncoding.size() == 0 ||
				 transferEncoding.back() == Header::TransferEncoding::IDENTITY)) {
				// body.inAvail must return -1 if no characters, 0 if indeterminate, or
				// an accurate number otherwise.
				std::streamsize inAvail{this->body.inAvail()};

				// Set Content-Length if body is determinate.
				if (inAvail == -1 && allowZero) {
					this->headers.contentLength(0);
				} else if (inAvail > 0) {
					this->headers.contentLength(static_cast<std::size_t>(inAvail));
				}
			}
		}

		// Process set default Content-Type if non-empty body.
		virtual void ppDefaultContentType() override {
			if (
				this->headers.contentLength() > 0 &&
				this->headers.find("Content-Type") == this->headers.end()) {
				this->headers.contentType(
					MediaType(MediaType::OCTET_STREAM, "charset=UTF-8"));
			}
		}

		protected:
		// Body parsing function shared between R/R.
		virtual void recvBody(std::istream &stream) override {
			// Create streambuf for body based on Transfer-Encoding and
			// Content-Length.
			std::vector<Header::TransferEncoding> transferEncoding;
			try {
				transferEncoding = this->headers.transferEncoding();
			} catch (...) {
				throw Exception(Error::MALFORMED_TRANSFER_ENCODING);
			}

			// Throw on any unsupported transfer encodings or if there are too many.
			if (transferEncoding.size() > (1_zu << 8)) {
				throw Exception(Error::TOO_MANY_TRANSFER_ENCODINGS);
			}
			for (Header::TransferEncoding transferEncodingSingle : transferEncoding) {
				switch (transferEncodingSingle) {
					case Header::TransferEncoding::IDENTITY:
					case Header::TransferEncoding::CHUNKED:
						break;
					default:
						throw Exception(Error::TRANSFER_ENCODING_NOT_SUPPORTED);
				}
			}

			// If empty, interpret as identity.
			if (transferEncoding.empty()) {
				transferEncoding.push_back(Header::TransferEncoding::IDENTITY);
			}

			// For each transfer encoding, pass it through the corresponding
			// streambuf.
			std::size_t contentLength;
			try {
				contentLength = this->headers.contentLength();
			} catch (...) {
				throw Exception(Error::MALFORMED_CONTENT_LENGTH);
			}

			// contentLength is 0 if doesn't exist.
			std::streambuf *curStreamBuf = stream.rdbuf();
			for (auto it = transferEncoding.rbegin(); it != transferEncoding.rend();
					 it++) {
				switch (*it) {
					case Header::TransferEncoding::IDENTITY:
						// No encoding beyond the first has a content-length.
						this->bodyTransferEncodingStreamBufs.emplace_back(
							new IdentityTransferEncodingIStreamBuf(
								curStreamBuf,
								it == transferEncoding.rbegin() ? contentLength : SIZE_MAX));

						break;
					case Header::TransferEncoding::CHUNKED:
						this->bodyTransferEncodingStreamBufs.emplace_back(
							new ChunkedTransferEncodingIStreamBuf(curStreamBuf));
						break;

					// No default.
					default:
						break;
				}

				// The streambuf from the previously processed encoding needs to be
				// updated.
				curStreamBuf = this->bodyTransferEncodingStreamBufs.back().get();
			}

			// Assign streambuf to body.
			this->body = Body(curStreamBuf);
		}
	};
}

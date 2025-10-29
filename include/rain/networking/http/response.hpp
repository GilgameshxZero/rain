// Response-specific HTTP parsing.
#pragma once

#include "../req-res/response.hpp"
#include "message.hpp"
#include "status-code.hpp"

namespace Rain::Networking::Http {
	class ResponseMessageSpecInterface
			: virtual public MessageSpecInterface,
				virtual public ReqRes::ResponseMessageSpecInterface {
		public:
		// Exception class carries over most errors from Message parsing.
		enum class Error {
			HTTP_VERSION_NOT_SUPPORTED = 1,
			MALFORMED_VERSION,
			MALFORMED_STATUS_CODE,
			MALFORMED_REASON_PHRASE,
			MALFORMED_HEADERS,
			MALFORMED_BODY
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Http::Response";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::HTTP_VERSION_NOT_SUPPORTED:
						return "HTTP version not supported.";
					case Error::MALFORMED_VERSION:
						return "Malformed HTTP version.";
					case Error::MALFORMED_STATUS_CODE:
						return "Malformed status code.";
					case Error::MALFORMED_REASON_PHRASE:
						return "Malformed reason phrase.";
					case Error::MALFORMED_HEADERS:
						return "Malformed HTTP headers.";
					case Error::MALFORMED_BODY:
						return "Malformed HTTP body, possibly due to Transfer-Encoding and "
									 "Content-Length.";
					default:
						return "Generic.";
				}
			}
		};
		using Exception = Rain::Error::Exception<Error, ErrorCategory>;

		protected:
		// Custom streambuf for HTTP/0.9 body parsing, which prioritizes the first
		// gcount bytes in the attempted version parsing.
		class Version_0_9BodyIStreamBuf : public std::streambuf {
			private:
			// The TCP Socket stream from which we will read from to fill up this
			// streambuf.
			std::streambuf *const sourceStreamBuf;

			// Leftover bytes from failed version parsing, to be sent first.
			std::string versionStr;

			// Internal buffer.
			std::string buffer;

			public:
			Version_0_9BodyIStreamBuf(
				std::streambuf *sourceStreamBuf,
				std::string const &versionStr,
				std::size_t bufferLen = 1_zu << 10)
					: sourceStreamBuf(sourceStreamBuf), versionStr(versionStr) {
				this->buffer.reserve(bufferLen);
				this->buffer.resize(this->buffer.capacity());

				// Set internal pointers for the versionStr.
				this->setg(
					&this->versionStr[0],
					&this->versionStr[0],
					&this->versionStr[0] + this->versionStr.length());
			}

			// Disable copy.
			Version_0_9BodyIStreamBuf(Version_0_9BodyIStreamBuf const &) = delete;
			Version_0_9BodyIStreamBuf &operator=(Version_0_9BodyIStreamBuf const &) =
				delete;

			protected:
			// Re-fill the buffer from the source, decrementing contentLength if
			// necessary.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					// Refill buffer directly from sourceStreamBuf.
					std::streamsize cFilled(this->sourceStreamBuf->sgetn(
						&this->buffer[0], this->buffer.length()));

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
	};

	template <typename Message>
	class ResponseMessageSpec : public Message,
															virtual public ResponseMessageSpecInterface {
		public:
		// Three-digit status code from RFC 2616.
		StatusCode statusCode;

		// Human-readable reason phrase.
		std::string reasonPhrase;

		private:
		std::unique_ptr<std::streambuf> version_0_9BodyIStreamBuf;

		public:
		// Carry over constructor which recvs from a Socket.
		ResponseMessageSpec(
			StatusCode statusCode = {},
			Headers &&headers = {},
			Body &&body = {},
			// An empty reason phrase will use the default one for the statusCode.
			std::string const &reasonPhrase = {},
			Version version = {})
				: Message(
						// bind version to rvalue reference to be perfect forwarded by
						// SuperInterface to Message.
						std::move(headers),
						std::move(body),
						version),
					statusCode(statusCode),
					reasonPhrase(reasonPhrase) {}
		ResponseMessageSpec(ResponseMessageSpec &&other)
				: Message(std::move(other)),
					statusCode(other.statusCode),
					reasonPhrase(std::move(other.reasonPhrase)),
					version_0_9BodyIStreamBuf(
						std::move(other.version_0_9BodyIStreamBuf)) {}

		// Overrides for Super versions implement protocol behavior.
		virtual void sendWith(std::ostream &stream) override {
			// pp-chain.
			this->ppEstimateContentLength(true);
			this->ppDefaultContentType();

			switch (this->version) {
				case Version::_1_0:
				case Version::_1_1:
					// Send startline/response line.
					stream << "HTTP/" << this->version << " " << this->statusCode << " "
								 << (this->reasonPhrase.empty()
											 ? this->statusCode.getReasonPhrase()
											 : this->reasonPhrase)
								 << "\r\n";

					// Send headers.
					stream << this->headers << "\r\n";
					break;

				case Version::_0_9:
					break;

				default:
					throw Exception(Error::HTTP_VERSION_NOT_SUPPORTED);
			}

			// HTTP/0.9, 1.0, 1.1 responses all have bodies.
			stream << this->body;
			stream.flush();
		}
		virtual void recvWith(std::istream &stream) override {
			// Version. Same as in Request, reserve 11 characters.
			std::string versionStr(11, ' ');
			try {
				stream.getline(&versionStr[0], versionStr.length(), ' ');

				// Create a copy of versionStr so that the original remains unchanged,
				// in case the parsing below throws. Then, the version is 0.9, and we
				// need to use the bytes in versionStr again as the body.
				std::string versionStrCopy{versionStr};
				versionStrCopy.resize(static_cast<std::size_t>(
					std::max(std::streamsize(0), stream.gcount() - 1)));

				// versionStr must be in the format HTTP/-.-. Otherwise, this throws,
				// and we assume HTTP/0.9.
				this->version = Version(versionStrCopy.substr(5));
			} catch (...) {
				// Assume that we are HTTP/0.9.
				this->version = Version::_0_9;

				// The body contains the first gcount bytes in versionStr. Regardless of
				// whether failbit is set, the final of those gcount bytes will be
				// correct from the space-init of versionStr.
				versionStr.resize(static_cast<std::size_t>(stream.gcount()));
				this->version_0_9BodyIStreamBuf.reset(
					new Version_0_9BodyIStreamBuf(stream.rdbuf(), versionStr));
				this->body = Body(this->version_0_9BodyIStreamBuf.get());

				return;
			}

			// Version successfully parsed (and dealt with if 0.9). Throw if still
			// unexpected.
			switch (this->version) {
				case Version::_0_9:
					// Version 0.9 does not have a startline.
					throw Exception(Error::MALFORMED_VERSION);

				case Version::_1_0:
				case Version::_1_1:
					break;

				default:
					throw Exception(Error::HTTP_VERSION_NOT_SUPPORTED);
			}

			// We must be in version 1.0 or 1.1. Get the rest of the startline.

			// Status code.
			try {
				std::string statusCodeStr(5, '\0');
				stream.getline(&statusCodeStr[0], statusCodeStr.length(), ' ');
				statusCodeStr.resize(static_cast<std::size_t>(
					std::max(std::streamsize(0), stream.gcount() - 1)));
				this->statusCode = StatusCode(statusCodeStr);
			} catch (...) {
				throw Exception(Error::MALFORMED_STATUS_CODE);
			}

			// Reason phrase.
			try {
				this->reasonPhrase.resize(1_zu << 10);
				stream.getline(&this->reasonPhrase[0], this->reasonPhrase.length());

				// Ignore the terminator nullptr as well as the \r.
				this->reasonPhrase.resize(static_cast<std::size_t>(
					std::max(std::streamsize(0), stream.gcount() - 2)));
			} catch (...) {
				throw Exception(Error::MALFORMED_REASON_PHRASE);
			}

			// Headers.
			try {
				stream >> this->headers;
			} catch (...) {
				throw Exception(Error::MALFORMED_HEADERS);
			}

			// Similar to request parsing, the body termination is determined by a
			// combination of Content-Length and Transfer-Encoding.
			try {
				this->recvBody(stream);
			} catch (...) {
				throw Exception(Error::MALFORMED_BODY);
			}
		}
	};

	// Shorthand.
	class Response : public ResponseMessageSpec<MessageSpec<ReqRes::Response>> {
		using ResponseMessageSpec<
			MessageSpec<ReqRes::Response>>::ResponseMessageSpec;
	};
}

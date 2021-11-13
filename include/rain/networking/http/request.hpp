// Request-specific HTTP parsing.
#pragma once

#include "../req-res/request.hpp"
#include "message.hpp"
#include "method.hpp"

namespace Rain::Networking::Http {
	class RequestMessageSpecInterface
			: virtual public MessageSpecInterface,
				virtual public ReqRes::RequestMessageSpecInterface {
		public:
		// Exception class required for catching from R/R onWork.
		enum class Error {
			HTTP_VERSION_NOT_SUPPORTED = 1,
			METHOD_NOT_ALLOWED,
			MALFORMED_VERSION,
			MALFORMED_HEADERS,
			MALFORMED_BODY
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Http::Request";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::HTTP_VERSION_NOT_SUPPORTED:
						return "HTTP version not supported.";
					case Error::METHOD_NOT_ALLOWED:
						return "Request method not allowed.";
					case Error::MALFORMED_VERSION:
						return "Malformed HTTP version.";
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
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;
	};

	template <typename Message>
	class RequestMessageSpec : public Message,
														 virtual public RequestMessageSpecInterface {
		public:
		// Method/verb of request.
		Method method;

		// Target URI (including query parameters and fragment).
		std::string target;

		// Carry over constructor which recvs from a Socket.
		RequestMessageSpec(
			Method method = {},
			std::string const &target = "/",
			Headers &&headers = {},
			Body &&body = {},
			Version version = {})
				: Message(
						// bind version to rvalue reference to be perfect forwarded by
						// SuperInterface to Message.
						std::move(headers),
						std::move(body),
						version),
					method(method),
					target(target) {}
		RequestMessageSpec(RequestMessageSpec &&other)
				: Message(std::move(other)),
					method(other.method),
					target(std::move(other.target)) {}

		// Overrides for Super versions implement protocol behavior.
		//
		// Calls (almost) no-op Message sendWith/recvWith to check transmissability.
		virtual void sendWith(std::ostream &stream) override {
			// pp-chain.
			this->ppEstimateContentLength(false);
			this->ppDefaultContentType();

			// Startline/request line.
			switch (this->version) {
				case Version::_1_0:
				case Version::_1_1:
					stream << this->method << " " << this->target << " HTTP/"
								 << this->version << "\r\n";
					stream << this->headers << "\r\n" << this->body;
					break;

				case Version::_0_9:
					stream << this->method << " " << this->target << "\r\n";

					// HTTP/0.9 Requests don't have headers nor body.
					break;

				default:
					throw Exception(Error::HTTP_VERSION_NOT_SUPPORTED);
			}

			stream.flush();
		}
		virtual void recvWith(std::istream &stream) override {
			// Receive startline. Take care to limit token length to avoid suffering
			// from maliciously long tokens.
			try {
				stream >> this->method;
			} catch (...) {
				throw Exception(Error::METHOD_NOT_ALLOWED);
			}

			// Target. May be terminated with \n instead of ' ' in the case of
			// HTTP/0.9.
			this->target.resize(1_zu << 12);
			stream.getline(&this->target[0], this->target.size(), '\n');

			// -2 for \r\0.
			this->target.resize(static_cast<std::size_t>(
				std::max(std::streamsize(0), stream.gcount() - 2)));

			// Version. Response version requires special parsing, so this can't be
			// abstracted out.
			try {
				// versionStr is 8 characters (e.g. HTTP/1.1), so we reserve +3 for the
				// nullptr and \r, and delimiter.
				// versionStr must be in the format HTTP/-.-.
				this->version = Version(this->target.substr(this->target.length() - 3));

				// Remove the version from the target.
				this->target.resize(
					std::min(this->target.length(), this->target.length() - 9));
			} catch (...) {
				// Bad version, assume it is 0.9, and the target remains the same.
				this->version = Version::_0_9;
			}

			// Receive headers & body.
			switch (this->version) {
				case Version::_1_0:
				case Version::_1_1: {
					try {
						stream >> this->headers;
					} catch (...) {
						throw Exception(Error::MALFORMED_HEADERS);
					}

					try {
						this->recvBody(stream);
					} catch (...) {
						throw Exception(Error::MALFORMED_BODY);
					}
					break;
				}

				case Version::_0_9:
					// No headers or body.
					break;

				default:
					throw Exception(Error::HTTP_VERSION_NOT_SUPPORTED);
			}
		}
	};

	// Shorthand.
	class Request : public RequestMessageSpec<MessageSpec<ReqRes::Request>> {
		using RequestMessageSpec<MessageSpec<ReqRes::Request>>::RequestMessageSpec;
	};
}

// HTTP headers.
#pragma once

#include "../../algorithm/algorithm.hpp"
#include "../../error/exception.hpp"
#include "../../literal.hpp"
#include "../../string/string.hpp"
#include "../host.hpp"
#include "../media-type.hpp"
#include "header.hpp"

#include <unordered_map>

namespace Rain::Networking::Http {
	// HTTP headers. Each header value is stored as std::any. Common headers are
	// strongly typed with access methods. Other headers are untyped and must be
	// accessed in the same way they were set. Do NOT set different types for
	// common headers than the intended.
	//
	// Subclasses std::map, but might as well be std::unordered_map with case
	// agnostic keys.
	//
	// Some headers are implemented as custom types/enums.
	class Headers : public std::unordered_map<
										std::string,
										std::string,
										String::HashCaseAgnostic,
										String::EqualCaseAgnostic> {
		public:
		enum class Error { NO_COLON_DELIMITER = 1, HEADERS_BLOCK_OVERFLOW };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Http::Headers";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::NO_COLON_DELIMITER:
						return "No colon delimiter in header line.";
					case Error::HEADERS_BLOCK_OVERFLOW:
						return "Headers cannot exceed 64KB in total.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		typedef std::unordered_map<
			std::string,
			std::string,
			String::HashCaseAgnostic,
			String::EqualCaseAgnostic>
			Super;

		Headers() = default;

		// Build inline with initializer list. Also serves as move constructor.
		Headers(Super &&headers) : Super(std::move(headers)) {}

		// Typed get/set for common headers. Get will return a default (defined per
		// header) value if header doesn't exist, and not create it.

		// Invalid value for contentLength is SIZE_MAX.
		std::size_t contentLength() {
			auto it = this->find("Content-Length");
			return it == this->end() ? 0 : std::stoull(it->second);
		}
		void contentLength(std::size_t value) {
			this->operator[]("Content-Length") = std::to_string(value);
		}

		MediaType contentType() {
			auto it = this->find("Content-Type");
			return it == this->end() ? MediaType() : MediaType(it->second);
		}
		void contentType(MediaType const &value) {
			this->operator[]("Content-Type") = static_cast<std::string>(value);
		}

		Host host() {
			auto it = this->find("Host");
			return it == this->end() ? Host() : Host(it->second);
		}
		void host(Host const &value) { this->operator[]("Host") = value; }

		std::string server() { return this->find("Server")->second; }
		void server(std::string const &value) {
			this->operator[]("Server") = value;
		}

		std::vector<Header::TransferEncoding> transferEncoding() {
			auto it = this->find("Transfer-Encoding");
			if (it == this->end()) {
				return {};
			}

			std::stringstream stream(this->operator[]("Transfer-Encoding"));

			std::vector<Header::TransferEncoding> transferEncoding;
			std::string transferEncodingSingle;
			while (std::getline(stream, transferEncodingSingle, ',')) {
				transferEncoding.emplace_back(
					String::trimWhitespace(transferEncodingSingle));
			}

			return transferEncoding;
		}
		void transferEncoding(std::vector<Header::TransferEncoding> const &value) {
			if (value.empty()) {
				return;
			}

			std::string &transferEncoding = this->operator[]("Transfer-Encoding");

			for (Header::TransferEncoding const &transferEncodingSingle : value) {
				transferEncoding +=
					static_cast<std::string>(transferEncodingSingle) + ", ";
			}
			transferEncoding.pop_back();
			transferEncoding.pop_back();
		}
	};
}

// Stream operators.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Http::Headers const &headers) {
	for (std::pair<std::string const, std::string> const &keyValue : headers) {
		stream << keyValue.first << ": " << keyValue.second << "\r\n";
	}
	return stream;
}
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Networking::Http::Headers &headers) {
	// TODO: Optimize for string copying.

	using namespace Rain::Literal;

	// HTTP header lines can be up to 4K long.
	// Total size of headers cannot exceed 64K.
	std::size_t totalHeadersBytes = 0;
	std::string line(1_zu << 12, '\0');
	while (stream.getline(&line[0], line.length())) {
		line.resize(std::max(std::streamsize(0), stream.gcount() - 1));

		// If line is only \r, we are done.
		if (line == "\r") {
			break;
		}

		// Copy header value to another string.
		std::size_t colonPos = line.find(':');

		if (colonPos == std::string::npos) {
			// Failed to find colon, malformed.
			throw Rain::Networking::Http::Headers::Exception(
				Rain::Networking::Http::Headers::Error::NO_COLON_DELIMITER);
		}

		std::string value(line.substr(colonPos + 1));

		// Resize line to only include the header name.
		line.resize(colonPos);

		// Alias with name and trim whitespace.
		std::string &name = line;
		Rain::String::trimWhitespace(name);
		Rain::String::trimWhitespace(value);

		// Add to headers.
		headers[name] = value;
		totalHeadersBytes += name.length() + value.length();
		if (totalHeadersBytes > (1_zu << 16)) {
			throw Rain::Networking::Http::Headers::Exception(
				Rain::Networking::Http::Headers::Error::HEADERS_BLOCK_OVERFLOW);
		}

		// Prepare for next line.
		line.resize(1_zu << 12);
	}

	return stream;
}

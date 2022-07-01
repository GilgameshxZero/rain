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
	class Headers : public std::unordered_multimap<
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

		typedef std::unordered_multimap<
			std::string,
			std::string,
			String::HashCaseAgnostic,
			String::EqualCaseAgnostic>
			Super;

		Headers() = default;

		// Build inline with initializer list. Also serves as move constructor.
		Headers(Super &&headers) : Super(std::move(headers)) {}

		// Retrieve the reference to a header value. A new header is created if it
		// does not already exist. A header which occurs multiple times is not
		// guaranteed consistent behavior.
		std::string &operator[](std::string const &key) {
			auto it = this->find(key);
			if (it == this->end()) {
				return this->insert({key, ""})->second;
			}
			return it->second;
		}

		// Typed get/set for common headers. Get will return a default (defined per
		// header) value if header doesn't exist, and NOT create it.

		// Invalid value for contentLength is SIZE_MAX.
		std::size_t contentLength() {
			auto it = this->find("Content-Length");
			return it == this->end() ? 0_zu
															 : static_cast<std::size_t>(std::strtoumax(
																	 it->second.c_str(), nullptr, 10));
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

		std::unordered_map<std::string, std::string> cookie() {
			auto it = this->find("Cookie");
			if (it == this->end()) {
				return {};
			}
			std::string const &cookieStr = it->second;

			std::size_t offset{0}, equals, semicolon{0};
			std::unordered_map<std::string, std::string> cookies;
			while (semicolon != std::string::npos) {
				equals = cookieStr.find('=', offset);
				semicolon = cookieStr.find(';', equals + 1);
				std::string name{cookieStr.substr(offset, equals - offset)},
					value{cookieStr.substr(equals + 1, semicolon - equals - 1)};
				Rain::String::trimWhitespace(name);
				Rain::String::trimWhitespace(value);
				cookies.insert({name, value});
				// Whitespace may follow the semicolon.
				offset = semicolon + 1;
			};
			return cookies;
		}
		void cookie(std::unordered_map<std::string, std::string> const &value) {
			std::string &cookieStr = this->operator[]("Cookie");
			for (auto const &i : value) {
				cookieStr += i.first + "=" + i.second + ";";
			}
			cookieStr.pop_back();
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

		std::unordered_map<std::string, Header::SetCookie> setCookie() {
			std::unordered_map<std::string, Header::SetCookie> result;
			auto equalRange{this->equal_range("Set-Cookie")};
			for (auto it = equalRange.first; it != equalRange.second; it++) {
				std::size_t offset = 0, equals = it->second.find('=', offset),
										semicolon = it->second.find(';', equals + 1);
				auto cookie{
					result
						.insert(
							{it->second.substr(offset, equals),
							 {it->second.substr(equals + 1, semicolon - equals - 1)}})
						.first};
				while (semicolon != std::string::npos) {
					// Whitespace may follow the semicolon.
					offset = semicolon + 1;
					std::string attribute, value;
					equals = it->second.find('=', offset);
					// Some attributes don’t have values i.e. Secure, HttpOnly.
					if (equals == std::string::npos) {
						semicolon = it->second.find(';', offset);
						attribute = it->second.substr(offset, equals);
					} else {
						semicolon = it->second.find(';', equals + 1);
						attribute = it->second.substr(offset, equals);
						value = it->second.substr(equals + 1, semicolon - equals - 1);
					}
					Rain::String::trimWhitespace(attribute);
					Rain::String::trimWhitespace(value);
					cookie->second.attributes.insert({attribute, value});
				}
			}
			return result;
		}
		void setCookie(
			std::unordered_map<std::string, Header::SetCookie> const &value) {
			this->erase("Set-Cookie");
			for (auto const &it : value) {
				std::string cookieStr{it.first + "=" + it.second.value};
				for (auto const &attribute : it.second.attributes) {
					cookieStr += "; ";
					cookieStr += attribute.first;
					if (!attribute.second.empty()) {
						cookieStr += "=" + attribute.second;
					}
				}
				this->insert({"Set-Cookie", cookieStr});
			}
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
	std::size_t totalHeadersBytes{0};
	std::string line(1_zu << 12, '\0');
	while (stream.getline(&line[0], line.length())) {
		line.resize(static_cast<std::size_t>(
			std::max(std::streamsize(0), stream.gcount() - 1)));

		// If line is only \r, we are done.
		if (line == "\r") {
			break;
		}

		// Copy header value to another string.
		std::size_t colonPos{line.find(':')};

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
		headers.insert({name, value});
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

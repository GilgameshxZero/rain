// Type for the Transfer-Encoding header.
#pragma once

#include "../../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Http::Header {
	class TransferEncoding {
		public:
		// Unscoped enum allows for direct name resolution from wrapper class.
		enum Value { IDENTITY = 0, CHUNKED, GZIP, DEFLATE, COMPRESS };
		Value const value;

		// Maps strings to Value.
		static inline std::unordered_map<std::string, Value> const fromStr{
			{"identity", IDENTITY},
			{"chunked", CHUNKED},
			{"gzip", GZIP},
			{"x-gzip", GZIP},
			{"deflate", DEFLATE},
			{"compress", COMPRESS}};

		public:
		// Construct directly or from string. May throw.
		constexpr TransferEncoding(Value value = IDENTITY) noexcept
				: value(value) {}
		TransferEncoding(std::string const &str)
				: value(TransferEncoding::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case IDENTITY:
					return "identity";
				case CHUNKED:
					return "chunked";
				case GZIP:
					return "gzip";
				case DEFLATE:
					return "deflate";
				case COMPRESS:
				// Default never occurs.
				default:
					return "compress";
			}
		}
	};
}

// Stream operator.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Http::Header::TransferEncoding transferEncoding) {
	return stream << static_cast<std::string>(transferEncoding);
}

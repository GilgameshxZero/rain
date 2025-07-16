// HTTP request method.
#pragma once

#ifdef DELETE
#undef DELETE
#endif

#include "../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Http {
	// HTTP method (GET, POST, etc.).
	class Method {
		public:
		// Unscoped enum allows for direct name resolution from wrapper class.
		enum Value {
			GET = 0,
			HEAD,
			POST,
			PUT,
			DELETE,
			CONNECT,
			OPTIONS,
			TRACE,
			PATCH
		};

		private:
		// Keep private to enforce value as one of the possible enum values.
		Value value;

		public:
		// Maps strings to Value.
		static inline std::unordered_map<std::string, Value> const fromStr{
			{"GET", GET},
			{"HEAD", HEAD},
			{"POST", POST},
			{"PUT", PUT},
			{"DELETE", DELETE},
			{"CONNECT", CONNECT},
			{"OPTIONS", OPTIONS},
			{"TRACE", TRACE},
			{"PATCH", PATCH}};

		// Direct constructors. Parsing strings may throw.
		constexpr Method(Value value = GET) noexcept : value(value) {}
		Method(std::string const &str) : value(Method::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case GET:
					return "GET";
				case HEAD:
					return "HEAD";
				case POST:
					return "POST";
				case PUT:
					return "PUT";
				case DELETE:
					return "DELETE";
				case CONNECT:
					return "CONNECT";
				case OPTIONS:
					return "OPTIONS";
				case TRACE:
					return "TRACE";
				case PATCH:
				// Default never occurs since value is private.
				default:
					return "PATCH";
			}
		}
	};
}

// Stream operators.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Http::Method method) {
	return stream << static_cast<std::string>(method);
}
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Networking::Http::Method &method) {
	// Any stream-in operations should not be subject to failure from a long line.
	//
	// Longest method is 7 characters, CONNECT/OPTIONS. Reserve +2 characters, one
	// for the terminating nullptr, one for reading the delimiter. failbit will
	// not be set if the maximum length is reached.
	std::string methodStr(9, '\0');
	stream.getline(&methodStr[0], methodStr.size(), ' ');
	methodStr.resize(static_cast<std::size_t>(
		std::max(std::streamsize(0), stream.gcount() - 1)));
	method = Rain::Networking::Http::Method(methodStr);
	return stream;
}

// SMTP AUTH method.
#pragma once

#include "../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Smtp {
	// SMTP AUTH method (PLAIN, LOGIN, etc.).
	class AuthMethod {
		public:
		// Unscoped enum allows for direct name resolution from wrapper class.
		enum Value { PLAIN = 0, LOGIN, CRAM_MD5 };
		Value value;

		// Maps strings to Value.
		inline static std::unordered_map<
			std::string,
			Value,
			String::HashCaseAgnostic,
			String::EqualCaseAgnostic> const fromStr{
			{"PLAIN", PLAIN},
			{"LOGIN", LOGIN},
			{"CRAM-MD5", CRAM_MD5}};

		public:
		// Direct constructors.
		constexpr AuthMethod(Value value = LOGIN) noexcept : value(value) {}
		AuthMethod(std::string const &str) : value(AuthMethod::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case PLAIN:
					return "PLAIN";
				case LOGIN:
					return "LOGIN";
				case CRAM_MD5:
				// Default never occurs.
				default:
					return "CRAM-MD5";
			}
		}
	};
}

// Stream operators. istream operator>> will take a token in the format
// HTTP/-.-.
std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Smtp::AuthMethod authMethod) {
	return stream << static_cast<std::string>(authMethod);
}

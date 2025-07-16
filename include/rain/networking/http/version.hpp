// HTTP version.
#pragma once

#include "../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Http {
	// HTTP version (0.9, 1.0, 1.1, etc.).
	class Version {
		public:
		// Unscoped enum allows for direct name resolution from wrapper class.
		enum Value { _0_9 = 0, _1_0, _1_1, _2_0, _3_0 };
		Value value;

		// Maps strings to Value.
		static inline std::unordered_map<std::string, Value> const fromStr{
			{"0.9", _0_9},
			{"1.0", _1_0},
			{"1.1", _1_1},
			{"2.0", _2_0},
			{"3.0", _3_0}};

		public:
		// Direct constructors.
		constexpr Version(Value value = _1_1) noexcept : value(value) {}
		Version(std::string const &str) : value(Version::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case _0_9:
					return "0.9";
				case _1_0:
					return "1.0";
				case _1_1:
					return "1.1";
				case _2_0:
					return "2.0";
				case _3_0:
				// Default never occurs.
				default:
					return "3.0";
			}
		}
	};
}

// Stream operators. istream operator>> will take a token in the format
// HTTP/-.-.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Http::Version version) {
	return stream << static_cast<std::string>(version);
}

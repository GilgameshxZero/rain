// Parses a command line with the KeyedParser as backbone.
#pragma once

#include "../error/exception.hpp"
#include "keyed-parser.hpp"

#include <set>

namespace Rain::String {
	class CommandLineParser : public KeyedParser {
		public:
		enum class Error : int {
			INVALID_KEY_PREFIX = 1,
			EMPTY_KEY,
			KEY_BUFFER_OVERFLOW
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::String::CommandLineParser";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::INVALID_KEY_PREFIX:
						return "Argument key name must begin with \"-\".";
					case Error::EMPTY_KEY:
						return "Argument key name is empty.";
					case Error::KEY_BUFFER_OVERFLOW:
						return "Key name is longer than buffer size.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		// Parse a list of command-line arguments. Do not pass the first argument
		// (typically filename) into this parser. Command-line arguments come in
		// key-value pairs. Keys longer than 1 character must be prefixed with --.
		// Values must come after keys, but optionally connected with the key with a
		// =. Short keys must be 1 character long and prefixed with -, with their
		// values optionally coming immediately after the key. Returns true on any
		// parser error. Throws on other errors.
		bool parse(int argc, char const *const *const argv) {
			// Key name buffer.
			std::string keyName;
			for (int a = 0; a < argc; a++) {
				// Current argument.
				char const *arg = argv[a],
									 // Candidate for value to be parsed.
					*value;

				if (arg[0] != '-') {
					// Both long and short keys must start with -.
					throw Exception(Error::INVALID_KEY_PREFIX);
				}
				if (arg[1] == '-') {
					// Long key name. Find delimiter.
					for (value = arg + 2; *value != '\0' && *value != '='; value++)
						;

					if (*value == '\0') {
						// No explicit delimiter, so value must be next argument.
						// Copy key name out.
						keyName = std::string(arg + 2, value - arg - 2);

						// If next argument begins with - and is a key, then this key is a
						// flag (without value). Otherwise, next argument is the value for
						// this key.
						value =
							(a + 1 == argc || argv[a + 1][0] == '-') ? nullptr : argv[++a];
					} else {
						// An explicit = delimiter exists, and the value is after that.
						keyName = std::string(arg + 2, value++ - arg - 2);
					}
				} else if (arg[1] == '\0') {
					// Short key must not be shorter than 1 character.
					throw Exception(Error::EMPTY_KEY);
				} else {
					// Short key.
					keyName = arg[1];

					if (arg[2] == '\0') {
						// Argument ends, so value is next argument. Again, check if the key
						// is a flag.
						value =
							(a + 1 == argc || argv[a + 1][0] == '-') ? nullptr : argv[++a];
					} else {
						// Value is concatenated to key.
						value = arg + 2;
					}
				}

				// Flags are explicitly parsed as TRUE and should be set by the caller
				// as bool stores.
				if (value == nullptr) {
					value = "1";
				}
				if (KeyedParser::parse(keyName, value)) {
					return true;
				}
			}

			return false;
		}
	};
}

#pragma once

#include "waterfall-parser.hpp"

namespace Rain::String {
	class CommandLineParser : public WaterfallParser {
		public:
		// Custom `error_condition`s.
		enum class Error { INVALID_KEY_PREFIX = 0, EMPTY_KEY };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::String::CommandLineParser";
			}
			std::string message(int ev) const noexcept {
				switch (static_cast<Error>(ev)) {
					case Error::INVALID_KEY_PREFIX:
						return "Argument key name must begin with \"-\".";
					case Error::EMPTY_KEY:
						return "Argument key name is empty.";
					default:
						return "Generic.";
				}
			}
		};

		// Parse a list of command-line arguments, without the first argument.
		void parse(int argc, char const *const *const argv) const {
			char argNameBuf[128];	 // Argument name to be passed to parse.
			for (int a = 0; a < argc; a++) {
				char const *arg = argv[a],	// Current argument.
					*value;	 // Argument value to be passed to parse.

				if (arg[0] != '-') {
					// Argument keys must start with -.
					throw ErrorException::Exception::makeException<ErrorCategory>(Error::INVALID_KEY_PREFIX);
				}
				if (arg[1] == '-') {	// Long argument name (beginning with --).
					// Find delimiter.
					for (value = arg + 2; *value != '\0' && *value != '='; value++)
						;
					if (*value ==
						'\0') {	 // No delimiter, value is next argument or none.
						strncpy_s(argNameBuf, sizeof(argNameBuf), arg + 2, value - arg - 2);
						value = (a + 1 == argc || argv[a + 1][0] == '-') ? "" : argv[++a];
					} else {	// Value is after = delimiter.
						strncpy_s(
							argNameBuf, sizeof(argNameBuf), arg + 2, value++ - arg - 2);
					}
				} else if (arg[1] == '\0') {
					// Argument keys must not be empty.
					throw ErrorException::Exception::makeException<ErrorCategory>(Error::EMPTY_KEY);
				} else {	// Short argument name (must be single letter!).
					if (arg[2] == '\0') {
						// Next argument is the value.
						value = (a + 1 == argc || argv[a + 1][0] == '-') ? "" : argv[++a];
					} else {
						value = arg + 2;
					}

					argNameBuf[0] = arg[1];
					argNameBuf[1] = '\0';
				}

				// Pass to universal parser.
				WaterfallParser::parse(argNameBuf, value);
			}
		}
	};
}

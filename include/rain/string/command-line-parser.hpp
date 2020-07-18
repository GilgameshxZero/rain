#pragma once

#include "waterfall-parser.hpp"

namespace Rain::String {
	class CommandLineParser : public WaterfallParser {
		public:
		// Parse a list of command-line arguments, without the first argument.
		void parse(int argc, const char **argv) const {
			char argNameBuf[128];	 // Argument name to be passed to parse.
			for (int a = 0; a < argc; a++) {
				const char *arg = argv[a],	// Current argument.
					*value;	 // Argument value to be passed to parse.

				// Arguments start with -.
				if (arg[0] != '-') {
					throw std::invalid_argument(
						"Command line argument name must start with \"-\".");
				}
				if (arg[1] == '-') {	// Long argument name.
					// Find delimiter.
					for (value = arg + 2; *value != '\0' && *value != '='; value++)
						;
					if (*value == '\0') {	 // Value is next argument or none.
						strncpy_s(argNameBuf, 128, arg + 2, value - arg - 2);
						value = (a + 1 == argc || argv[a + 1][0] == '-') ? "\0" : argv[++a];
					} else {	// Value is after = delimiter.
						strncpy_s(argNameBuf, 128, arg + 2, value++ - arg - 2);
					}
					arg += 2;
				} else {	// Short argument name.
					if (arg[2] == '\0') {
						value = (a + 1 == argc || argv[a + 1][0] == '-') ? "\0" : argv[++a];
					} else {
						value = arg + 2;
					}

					// Move the arg around to have the right null terminators.
					argNameBuf[0] = arg[1];
					argNameBuf[1] = '\0';
				}
				WaterfallParser::parse(argNameBuf, value);
			}
		}
	};
}

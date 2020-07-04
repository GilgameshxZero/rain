#pragma once

#include "./string.hpp"

#include <map>

namespace Rain {
	class WaterfallParser {
		public:
		// Compound type to store all information with a parser.
		struct KeyHandler {
			// Function types.
			typedef int (*Parser)(const char *, void *);
			typedef void (*Destructor)(void *);

			// Constructor for inline initialization.
			KeyHandler(void *param, Parser parser, Destructor destructor)
					: param(param), parser(parser), destructor(destructor) {}

			// For parsing.
			void *param;
			Parser parser;

			// Frees the param if previously allocated.
			Destructor destructor;
		};

		~WaterfallParser() {
			// Free all dynamically allocated parameter memory.
			for (auto it = this->keyParsers.begin(); it != this->keyParsers.end();
					 it++) {
				it->second.destructor(it->second.param);
			}
		}

		// Attempt to parse a key with previously added parsers.
		int parse(const char *key, const char *sValue) const {
			// Error if key not found.
			auto it = this->keyParsers.find(key);
			if (it == this->keyParsers.end()) {
				return 1;
			}

			return it->second.parser(sValue, it->second.param);
		}

		// Add a way to parse a key. Key must not have been added before.
		int addParser(const char *key, bool *store) {
			return this->addParser(
				key,
				reinterpret_cast<void *>(store),
				[](const char *sValue, void *param) {
					*reinterpret_cast<bool *>(param) =
						!(strcmp(sValue, "0") == 0 || strcmp(sValue, "false") == 0 ||
							strcmp(sValue, "False") == 0 || strcmp(sValue, "FALSE") == 0);
					return 0;
				},
				[](void *param) {});
		}
		int addParser(const char *key, long long *store) {
			return this->addParser(
				key,
				reinterpret_cast<void *>(store),
				[](const char *sValue, void *param) {
					try {
						*reinterpret_cast<long long *>(param) = strtoll(sValue, NULL, 10);
						return 0;
					} catch (...) {
						return 1;
					}
				},
				[](void *param) {});
		}
		int addParser(const char *key, char *store, size_t sz) {
			return this->addParser(
				key,
				// Dynamically allocate a parameter containing both store and sz.
				reinterpret_cast<void *>(new std::pair<char *, size_t>(store, sz)),
				[](const char *sValue, void *param) {
					std::pair<char *, size_t> *origParam =
						reinterpret_cast<std::pair<char *, size_t> *>(param);
					strcpy_s(origParam->first, origParam->second, sValue);
					return 0;
				},
				[](void *param) {
					// And free it later.
					std::pair<char *, size_t> *origParam =
						reinterpret_cast<std::pair<char *, size_t> *>(param);
					delete origParam;
				});
		}
		int addParser(const char *key, std::string *store) {
			return this->addParser(
				key,
				// Dynamically allocate a parameter containing both store and sz.
				reinterpret_cast<void *>(store),
				[](const char *sValue, void *param) {
					reinterpret_cast<std::string *>(param)->assign(sValue);
					return 0;
				},
				[](void *param) {});
		}

		// Add a way to parse a key. Contains shared error-checking code and builds
		// the KeyHandler. Used as a shorthand by other addParser.
		int addParser(const char *key,
			void *param,
			WaterfallParser::KeyHandler::Parser parser,
			WaterfallParser::KeyHandler::Destructor destructor) {
			// Can't add a key that's been previously added.
			if (this->keyParsers.find(key) != this->keyParsers.end()) {
				return 1;
			}

			this->keyParsers.insert(std::make_pair(
				key, WaterfallParser::KeyHandler(param, parser, destructor)));
			return 0;
		}

		private:
		// All lambdas set by addKey.
		std::map<const char *, KeyHandler, cStrCmp> keyParsers;
	};

	class CommandLineParser : public WaterfallParser {
		public:
		int parse(int argc, char *argv[]) const {
			static char nullValue = '\0';
			for (int a = 0; a < argc; a++) {
				char *arg = argv[a], *value;

				// Arguments start with -.
				if (arg[0] != '-') {
					return 1;
				}

				if (arg[1] == '-') {
					// Long argument name.
					// Find delimiter.
					for (value = arg + 2; *value != '\0' && *value != '='; value++)
						;
					if (*value == '\0') {
						if (a + 1 == argc || argv[a + 1][0] == '-') {
							value = &nullValue;
						} else {
							value = argv[++a];
						}
					} else {
						*value = '\0';
						value++;
					}
					arg += 2;
				} else {
					// Short argument name.
					if (arg[2] == '\0') {
						if (a + 1 == argc || argv[a + 1][0] == '-') {
							value = &nullValue;
						} else {
							value = argv[++a];
						}
					} else {
						value = arg + 2;
					}

					// Move the arg around to have the right null terminators.
					arg[0] = arg[1];
					arg[1] = '\0';
				}
				if (value[0] == '-') {
					// The value is the next argument; current argument has no value.
					// It will be set to the null string, which will be parsed to true.
					a--;
					value = &nullValue;
				}
				WaterfallParser::parse(arg, value);
			}
			return 0;
		}
	};
}

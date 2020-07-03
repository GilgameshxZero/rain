#pragma once

#include "rain/string.hpp"

#include <map>

namespace Rain {
	struct cStrCmp {
		bool operator()(const char *x, const char *y) const {
			return std::strcmp(x, y) < 0;
		}
	};

	class WaterfallParser {
		public:
		struct KeyHandler {
			// Function types.
			typedef int (*Parser)(const char *, void *);
			typedef void (*Destructor)(void *);

			KeyHandler(void *, Parser, Destructor);

			void *param;
			Parser parser;

			// Frees the param if previously allocated.
			Destructor destructor;
		};

		~WaterfallParser();

		// Attempt to parse a key with previously added parsers.
		int parse(const char *, const char *) const;

		// Add a way to parse a key. Key must not have been added before.
		int addParser(const char *, bool *);
		int addParser(const char *, long long *);
		int addParser(const char *, char *, rsize_t);

		// Add a way to parse a key. Used as a shorthand by other addParser.
		int addParser(
			const char *,
			void *,
			KeyHandler::Parser,
			KeyHandler::Destructor);

		private:
		// All lambdas set by addKey.
		std::map<const char *, KeyHandler, cStrCmp> keyParsers;
	};

	class CommandLineParser : public WaterfallParser {
		public:
		int parse(int, char *[]) const;
	};
}

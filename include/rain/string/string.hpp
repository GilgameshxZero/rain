/*
A compatibility layer for different string implementations across platforms.

Includes common utility functions for strings.
*/

#pragma once

#include "../platform/.hpp"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace Rain {
	// An implementation of GNU strcpy_s.
	errno_t strcpy_s(char *dest, size_t destsz, const char *src) {
#ifdef RAIN_WINDOWS
		return ::strcpy_s(dest, destsz, src);
#else
		strncpy(dest, src, destsz)[destsz - 1] = '\0';
		return 0;
#endif
	}
}

namespace Rain::String {
	// Comparator for const char * for use in std::maps and like.
	struct cStrCmp {
		bool operator()(const char *x, const char *y) const {
			return strcmp(x, y) < 0;
		}
	};

	// Shorthand for std::string operations.
	std::string *strToLower(std::string *s) {
		for (size_t a = 0; a < s->length(); a++) {
			(*s)[a] = tolower((*s)[a]);
		}
		return s;
	}
	std::string *strTrimWhite(std::string *s) {
		s->erase(s->begin(), std::find_if(s->begin(), s->end(), [](int ch) {
			return !std::isspace(ch);
		}));
		s->erase(std::find_if(
							 s->rbegin(), s->rend(), [](int ch) { return !std::isspace(ch); })
							 .base(),
			s->end());
		return s;
	}

	// std::stringstream is pretty slow. Use other conversions if possible.
	template <typename T>
	std::string tToStr(T x) {
		std::stringstream ss;
		ss << x;
		return ss.str();
	}
	template <typename T>
	T strToT(std::string s) {
		T r;
		std::stringstream ss(s);
		ss >> r;
		return r;
	}
}

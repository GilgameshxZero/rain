/*
A compatibility layer for different string implementations across platforms.

Includes common utility functions for strings.
*/

#pragma once

// This defines _GNU_SOURCE if necessary for the string libraries.
#include "./platform.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include <string>

namespace Rain {
	typedef size_t rsize_t;
	typedef int errno_t;

	// Comparator for const char * for use in std::maps and like.
	struct cStrCmp {
		bool operator()(const char *x, const char *y) const {
			return strcmp(x, y) < 0;
		}
	};

	// An implementation of GNU memmem, strstr with explicit lengths.
	void *memmem(const void *haystack,
		size_t haystackLen,
		const void *const needle,
		const size_t needleLen) {
#ifdef RAIN_WINDOWS
		for (const char *h = reinterpret_cast<const char *>(haystack);
				 haystackLen >= needleLen;
				 ++h, --haystackLen) {
			if (!memcmp(h, needle, needleLen)) {
				return const_cast<void *>(reinterpret_cast<const void *>(h));
			}
		}
		return NULL;
#else
		return ::memmem(haystack, haystackLen, needle, needleLen);
#endif
	}

	// An implementation of GNU strcpy_s.
	errno_t strcpy_s(char *dest, size_t destsz, const char *src) {
#ifdef RAIN_WINDOWS
		return ::strcpy_s(dest, destsz, src);
#else
		strncpy(dest, src, destsz)[destsz - 1] = '\0';
		return 0;
#endif
	}

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

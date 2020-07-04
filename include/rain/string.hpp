/*
A compatibility layer for different string implementations across platforms.

Includes common utility functions for strings.
*/

#pragma once

// This defines _GNU_SOURCE if necessary for the string libraries.
#include "./platform.hpp"

#include <cassert>
#include <cstring>
#include <string>

namespace Rain {
	typedef size_t rsize_t;
	typedef int errno_t;

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
}

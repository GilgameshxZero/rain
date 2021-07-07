/*
Re-implementation of GNU memmem in the case it doesn't exist.
*/

#pragma once

#include "platform.hpp"

#include <cstring>

namespace Rain {
	// An implementation of GNU memmem, strstr with explicit lengths.
	inline void const *memmem(void const *const haystack,
		std::size_t haystackLen,
		void const *const needle,
		std::size_t needleLen) {
		for (char const *h = reinterpret_cast<char const *>(haystack);
				 haystackLen >= needleLen;
				 ++h, --haystackLen) {
			if (!memcmp(h, needle, needleLen)) {
				return reinterpret_cast<void const *>(h);
			}
		}
		return NULL;
	}
	inline void *memmem(void *const haystack,
		std::size_t haystackLen,
		void const *const needle,
		std::size_t needleLen) {
		return const_cast<void *>(memmem(static_cast<void const *>(haystack),
			haystackLen,
			needle,
			needleLen));
	}
}

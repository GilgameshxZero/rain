#pragma once

#include "../platform.hpp"
#include "../types.hpp"

#include <limits>

namespace Rain::Algorithm {
	// Compute partial match table (used in KMP) for a string. partialMatch must
	// be at least sLen + 1 in length.
	inline void computeKmpPartialMatch(const char *s,
		std::size_t sLen,
		std::size_t *partialMatch) {
		// This represents -1.
		partialMatch[0] = std::numeric_limits<std::size_t>::max();

		std::size_t candidate = 0;
		for (std::size_t a = 1; a < sLen; a++) {
			if (s[a] == s[candidate]) {
				partialMatch[a] = partialMatch[candidate];
			} else {
				partialMatch[a] = candidate;
				while (candidate != std::numeric_limits<std::size_t>::max() &&
					s[a] != s[candidate]) {
					candidate = partialMatch[candidate];
				}
			}
			candidate++;
		}

		partialMatch[sLen] = candidate;
	}

	// KMP string search while storing candidate state.
	inline char *cStrSearchKMP(const char *haystack,
		std::size_t haystackLen,
		const char *const needle,
		const std::size_t needleLen,
		const std::size_t *partialMatch,
		std::size_t *candidate) {
		for (std::size_t a = 0; a < haystackLen;) {
			if (haystack[a] == needle[*candidate]) {
				a++;
				(*candidate)++;
				if (static_cast<std::size_t>(*candidate) == needleLen) {
					return const_cast<char *>(haystack) + a - *candidate;
				}
			} else {
				*candidate = partialMatch[*candidate];
				if (*candidate == std::numeric_limits<std::size_t>::max()) {
					a++;
					(*candidate)++;
				}
			}
		}

		return NULL;
	}

	// Knuth-Morris-Pratt string search: an O(m+n) version of strstr.
	inline char *cStrSearchKMP(const char *haystack,
		std::size_t haystackLen,
		const char *const needle,
		const std::size_t needleLen) {
		std::size_t *partialMatch = new std::size_t[needleLen + 1];
		computeKmpPartialMatch(needle, needleLen, partialMatch);
		std::size_t candidate = 0;
		char *result = cStrSearchKMP(
			haystack, haystackLen, needle, needleLen, partialMatch, &candidate);
		delete[] partialMatch;
		return result;
	}
}

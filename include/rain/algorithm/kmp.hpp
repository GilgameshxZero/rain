#pragma once

#include "../algorithm.hpp"
#include "../platform.hpp"
#include "../type.hpp"

#include <limits>

namespace Rain::Algorithm {
	/*
	Compute partial match table (also known as the failure function) (used in KMP)
	for a string. partialMatch must be at least sLen + 1 in length.

	The partial match table specifies where to "rewind" the matching process to if
	it failes on a given character.
	*/
	inline void computeKmpPartialMatch(char const *const s,
		std::size_t sLen,
		std::size_t *partialMatch) {
		// This represents -1, i.e. we can resume matching for the first character
		// of the string at the next position in the text.
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

	// Overloads of computeKmpPartialMatch based for ease-of-use.

	// KMP string search while storing candidate state.
	inline char *cStrSearchKmp(char const *haystack,
		std::size_t haystackLen,
		char const *const needle,
		std::size_t const needleLen,
		std::size_t const *partialMatch,
		std::size_t *candidate) {
		for (std::size_t a = 0; a < haystackLen;) {
			if (haystack[a] == needle[*candidate]) {
				a++;
				(*candidate)++;
				if (*candidate == needleLen) {
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
	inline char *cStrSearchKmp(char const *haystack,
		std::size_t haystackLen,
		char const *const needle,
		std::size_t const needleLen) {
		std::size_t *partialMatch = new std::size_t[needleLen + 1];
		computeKmpPartialMatch(needle, needleLen, partialMatch);
		std::size_t candidate = 0;
		char *result = cStrSearchKmp(
			haystack, haystackLen, needle, needleLen, partialMatch, &candidate);
		delete[] partialMatch;
		return result;
	}
}

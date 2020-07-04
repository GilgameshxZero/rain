#pragma once

#include "./string.hpp"

namespace Rain {
	namespace Algorithm {
		// Compute partial match table (used in KMP) for a string. partialMatch must
		// be at least sLen + 1 in length.
		inline void partialMatchTable(const char *s,
			size_t sLen,
			long long *partialMatch) {
			partialMatch[0] = -1;

			long long candidate = 0;
			for (size_t a = 1; a < sLen; a++) {
				if (s[a] == s[candidate]) {
					partialMatch[a] = partialMatch[candidate];
				} else {
					partialMatch[a] = candidate;
					while (candidate >= 0 && s[a] != s[candidate]) {
						candidate = partialMatch[candidate];
					}
				}
				candidate++;
			}

			partialMatch[sLen] = candidate;
		}

		// KMP string search while storing candidate state.
		inline char *cStrSearchKMP(const char *haystack,
			size_t haystackLen,
			const char *const needle,
			const size_t needleLen,
			long long *partialMatch,
			long long *candidate) {

			for (size_t a = 0; a < haystackLen;) {
				if (haystack[a] == needle[*candidate]) {
					a++;
					(*candidate)++;
					if (static_cast<size_t>(*candidate) == needleLen) {
						return const_cast<char *>(haystack) + a - *candidate;
					}
				} else {
					*candidate = partialMatch[*candidate];
					if (*candidate < 0) {
						a++;
						(*candidate)++;
					}
				}
			}

			return NULL;
		}

		// Knuth-Morris-Pratt string search: an O(m+n) version of strstr.
		inline char *cStrSearchKMP(const char *haystack,
			size_t haystackLen,
			const char *const needle,
			const size_t needleLen) {
			long long *partialMatch = new long long[needleLen + 1];
			partialMatchTable(needle, needleLen, partialMatch);
			long long candidate = 0;
			char *result = cStrSearchKMP(
				haystack, haystackLen, needle, needleLen, partialMatch, &candidate);
			delete partialMatch;
			return result;
		}
	}
}

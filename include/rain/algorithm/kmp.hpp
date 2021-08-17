// Knuth-Morris-Pratt O(N+M) single perfect string-matching algorithm.
#pragma once

#include "../platform.hpp"
#include "../string/string.hpp"
#include "algorithm.hpp"

#include <limits>
#include <vector>

namespace Rain::Algorithm {
	// Compute partial match table (also known as the failure function) (used in
	// KMP) for a string.
	//
	// The partial match table specifies where to "rewind" the matching process to
	// if it failes on a given character.
	inline std::vector<std::size_t> computeKmpPartialMatch(
		char const *const cStr,
		std::size_t const cStrLen) {
		std::vector<std::size_t> partialMatch(cStrLen + 1);

		// This represents -1, i.e. we can resume matching for the first character
		// of the string at the next position in the text.
		partialMatch[0] = SIZE_MAX;

		// How far into search string s we must begin (to our best knowledge) if we
		// mismatch.
		std::size_t candidate = 0;

		for (std::size_t a = 1; a < cStrLen; a++) {
			if (cStr[a] == cStr[candidate]) {
				partialMatch[a] = partialMatch[candidate];
			} else {
				partialMatch[a] = candidate;
				while (candidate != SIZE_MAX && cStr[a] != cStr[candidate]) {
					candidate = partialMatch[candidate];
				}
			}
			candidate++;
		}

		partialMatch[cStrLen] = candidate;
		return partialMatch;
	}
	inline std::vector<std::size_t> computeKmpPartialMatch(std::string const &s) {
		return computeKmpPartialMatch(s.c_str(), s.length());
	}

	// KMP search for needle in haystack, in O(N+M). Optionally pass a candidate
	// parameter if want to continue from a previous search. Returns a pair of
	// (match, candidate). The match is nullptr if no match found, otherwise
	// returns a char * to the first character of the first match found. The
	// candidate is the updated candidate at the termination of the search.
	inline std::pair<char *, std::size_t> kmpSearch(
		char const *const haystack,
		std::size_t const haystackLen,
		char const *const needle,
		std::size_t const needleLen,
		std::vector<std::size_t> const &partialMatch = {},
		std::size_t candidate = 0) {
		// Use these variables, which are set based on whether or not default
		// partialMatch and candidate were passed in.
		std::vector<std::size_t> const &partialMatchProxy =
			partialMatch.size() == 0 ? computeKmpPartialMatch(needle) : partialMatch;

		char const *searchResult = nullptr;
		for (std::size_t a = 0; a < haystackLen;) {
			if (haystack[a] == needle[candidate]) {
				a++;
				candidate++;
				if (candidate == needleLen) {
					searchResult = haystack + a - candidate;
				}
			} else {
				// Use the partial match table to resume if a match fails.
				candidate = partialMatchProxy[candidate];
				if (candidate == SIZE_MAX) {
					a++;
					candidate = 0;
				}
			}
		}

		return std::make_pair(const_cast<char *>(searchResult), candidate);
	}
	inline std::pair<char *, std::size_t> kmpSearch(
		char const *const haystack,
		std::size_t haystackLen,
		std::string const &needle,
		std::vector<std::size_t> const &partialMatch = {},
		std::size_t candidate = 0) {
		return kmpSearch(
			haystack,
			haystackLen,
			needle.c_str(),
			needle.length(),
			partialMatch,
			candidate);
	}
	inline std::pair<std::size_t, std::size_t> kmpSearch(
		std::string const &haystack,
		char const *const needle,
		std::size_t const needleLen,
		std::vector<std::size_t> const &partialMatch = {},
		std::size_t candidate = 0) {
		std::pair<char *, std::size_t> searchRes = kmpSearch(
			haystack.c_str(),
			haystack.length(),
			needle,
			needleLen,
			partialMatch,
			candidate);
		return std::make_pair(searchRes.first - haystack.c_str(), searchRes.second);
	}
	inline std::pair<std::size_t, std::size_t> kmpSearch(
		std::string const &haystack,
		std::string const &needle,
		std::vector<std::size_t> const &partialMatch = {},
		std::size_t candidate = 0) {
		return kmpSearch(
			haystack, needle.c_str(), needle.length(), partialMatch, candidate);
	}
}

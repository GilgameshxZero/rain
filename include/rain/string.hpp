/*
A compatibility layer for different string implementations across platforms.
Includes some string-related utility functions.

Functionally replaces STL <string>
*/

#pragma once

#include "algorithm.hpp"
#include "platform.hpp"
#include "type.hpp"

#include <cstring>
#include <locale>
#include <sstream>
#include <string>

namespace Rain {
	// An implementation of GNU strcpy_s.
	inline errno_t strcpy_s(char *const dest,
		std::size_t destsz,
		char const *const src) {
#ifdef RAIN_WINDOWS
		return ::strncpy_s(dest, destsz, src, _TRUNCATE);
#else
		strncpy(dest, src, destsz)[destsz - 1] = '\0';
		return 0;
#endif
	}

	// An implementation of GNU strncpy_s.
	inline errno_t strncpy_s(char *const dest,
		std::size_t destsz,
		char const *const src,
		std::size_t count) {
		return strcpy_s(dest, min(destsz, count + 1), src);
	}
}

namespace Rain::String {
	// Comparator for const char * for use in std::maps and the like.
	struct cStrCmp {
		bool operator()(char const *const x, char const *const y) const {
			return strcmp(x, y) < 0;
		}
	};

	// Convert string to lowercase.
	inline std::string *toLowerStr(std::string *const str) {
		std::transform(str->begin(), str->end(), str->begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(static_cast<int>(c)));
		});
		return str;
	}
	inline char *toLowerCStr(char *const cStr) {
		for (char *it = cStr; *it; it++) {
			*it = static_cast<char>(std::tolower(static_cast<int>(*it)));
		}
		return cStr;
	}

	// Utilities for trimming whitespace on ends of string.
	inline std::string *trimWhitespaceStr(std::string *const str) {
		str->erase(
			str->begin(), std::find_if(str->begin(), str->end(), [](unsigned char c) {
				return std::isspace(static_cast<int>(c)) == 0;
			}));
		str->erase(
			std::find_if(str->rbegin(),
				str->rend(),
				[](unsigned char c) { return std::isspace(static_cast<int>(c)) == 0; })
				.base(),
			str->end());
		return str;
	}
	inline char const *findFirstNonWhitespaceCStrN(char const *cStr,
		std::size_t const cStrLen = 0,
		// direction = 1 specifies scanning LTR.
		std::size_t const direction = 1) {
		for (std::size_t a = 0; ((cStrLen == 0 && *cStr) || a < cStrLen) &&
				 std::isspace(static_cast<int>(*cStr));
				 cStr += direction, a++)
			;
		return cStr;
	}
	inline char *findFirstNonWhitespaceCStrN(char *cStr,
		std::size_t const cStrLen = 0,
		// direction = 1 specifies scanning LTR.
		std::size_t const direction = 1) {
		return const_cast<char *>(findFirstNonWhitespaceCStrN(
			static_cast<char const *>(cStr), cStrLen, direction));
	}

	// Convert from any type to another using std::stringstream.
	// This is fairly slow, so use more specialized methods if possible (strtoll,
	// to_string).
	template <typename ToType, typename FromType>
	ToType anyToAny(FromType from) {
		ToType to;
		std::stringstream ss;
		ss << from;
		ss >> to;
		return to;
	}
}

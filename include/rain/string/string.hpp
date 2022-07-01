// String-related utility functions.
//
// Eligible functions provide overloads in the form of C-String w/ explicit
// length and std::string parameterizations. Const parameters may return
// non-const pointers; the caller is responsible for tracking the const-ness of
// parameters.
//
// Out-of-bounds errors are left to the discretion of the caller.
//
// Functions support only ASCII-7 by assumption. Otherwise, strings are
// represented in UTF-8 internally. On Windows, the UNICODE preprocessor is
// defined, but all strings are converted to multibyte UTF-8 after kernel
// function processing.
#pragma once

#include "../algorithm/algorithm.hpp"
#include "../platform.hpp"
#include "../type.hpp"

#include <cinttypes>
#include <cstring>
#include <functional>
#include <locale>
#include <sstream>
#include <string>

namespace Rain {
	// strcasecmp does not exist on Windows.
	inline int strcasecmp(char const *left, char const *right) {
#ifdef RAIN_PLATFORM_WINDOWS
		return _stricmp(
#else
		return ::strcasecmp(
#endif
			left, right);
	}
}

namespace Rain::String {
	// Convert ASCII string types to lowercase.
	inline char *toLower(char *const cStr, std::size_t const cStrLen) {
		std::transform(cStr, cStr + cStrLen, cStr, [](unsigned char c) {
			return static_cast<char>(std::tolower(static_cast<int>(c)));
		});
		return cStr;
	}
	inline std::string &toLower(std::string &str) {
		// In C++17, strings are guaranteed to be continuous.
		toLower(&str[0], str.length());
		return str;
	}

	// Trim whitespace characters from both sides of an std::string.
	inline std::string &trimWhitespace(std::string &str) {
		str.erase(
			str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char c) {
				return std::isspace(static_cast<int>(c)) == 0;
			}));
		str.erase(
			std::find_if(
				str.rbegin(),
				str.rend(),
				[](unsigned char c) { return std::isspace(static_cast<int>(c)) == 0; })
				.base(),
			str.end());
		return str;
	}

	// For C-Strings, instead of trimming whitespace, these utilities find the
	// first/last non-whitespace character in the string.
	//
	// If all whitespace, returns cStr + cStrLen.
	inline char *scanUntilNonWhitespace(
		char const *const cStr,
		// Length in the direction to be scanned.
		std::size_t const cStrLen,
		// Scanning direction.
		bool const scanRight = true) {
		std::size_t index{0};
		for (; index < cStrLen && std::isspace(static_cast<int>(cStr[index]));
				 index += (scanRight ? 1 : -1))
			;
		return const_cast<char *>(cStr) + index;
	}
	inline char *scanUntilWhitespace(
		char const *const cStr,
		std::size_t const cStrLen,
		bool const scanRight = true) {
		std::size_t index{0};
		for (; index < cStrLen && !std::isspace(static_cast<int>(cStr[index]));
				 index += (scanRight ? 1 : -1))
			;
		return const_cast<char *>(cStr) + index;
	}

	// Convert any time to any other type using std::stringstream. Thread-safe.
	// Prefer specialized methods if available (strtoll, to_string, etc.).
	//
	// Internally, rain uses the strto* variants instead of the sto* variants as
	// the former returns valid results on error and do not throw.
	template <typename To, typename From>
	inline To anyToAny(From from) {
		To to;
		std::stringstream ss;
		ss << from;
		ss >> to;
		return to;
	}
	// Comparison operator for ASCII C-strings for use in std::maps and the like.
	struct CompareCString {
		bool operator()(char const *left, char const *right) const {
			return std::strcmp(left, right) < 0;
		}
	};

	// Comparison operator for case-agnostic comparison.
	struct CompareCaseAgnostic {
		bool operator()(std::string const &left, std::string const &right) const {
			return strcasecmp(left.c_str(), right.c_str()) < 0;
		}
	};

	// Case-agnostic hash & equality for std::unordered_map.
	struct HashCaseAgnostic {
		std::size_t operator()(std::string const &str) const {
			std::string strLower(str);
			return std::hash<std::string>{}(toLower(strLower));
		}
	};
	struct EqualCaseAgnostic {
		bool operator()(std::string const &left, std::string const &right) const {
			return strcasecmp(left.c_str(), right.c_str()) == 0;
		}
	};
}

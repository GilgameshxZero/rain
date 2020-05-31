/*
Various string-related helper functions.
*/

#pragma once

#include "platform.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace Rain {
	extern const std::string CRLF, LF;

	// Converts from any type to string using stringstream.
	template <typename T>
	std::string tToStr(T x) {
		std::stringstream ss;
		ss << x;
		return ss.str();
	}

	// Converts from a string to some other type using stringstream.
	template <typename T>
	T strToT(std::string s) {
		T r;
		std::stringstream ss(s);
		ss >> r;
		return r;
	}

	// Converts any type to a byte string.
	template <typename T>
	std::string tToByteStr(T x) {
		std::string ret;
		int xLen = sizeof x;
		char *xC = reinterpret_cast<char *>(static_cast<void *>(&x));
		for (int a = 0; a < xLen; a++)
			ret.push_back(xC[a]);
		return ret;
	}

	// Convenience function to convert to/from unicode/multibyte std::string.
	std::wstring mbStrToWStr(std::string *s);
	std::wstring mbStrToWStr(std::string s);
	std::string wStrToMBStr(std::wstring *ws);
	std::string wStrToMBStr(std::wstring ws);

	// Convert string to lowercase.
	std::string *strToLower(std::string *s);
	std::string strToLower(std::string s);

	// Trim whitespace from front and end of string.
	std::string *strTrimWhite(std::string *s);
	std::string strTrimWhite(std::string s);

	// Encodes and decodes base-64 format.
	char intEncodeB64(int x);
	int chrDecodeB64(char c);
	std::string strEncodeB64(const std::string *str);
	std::string strEncodeB64(std::string str);
	std::string strDecodeB64(const std::string *str);
	std::string strDecodeB64(std::string str);

	// Encode and decode from URI format.
	// Does not modify parameters if passed by pointer.
	// EncodeURL taken from
	// http://stackoverflow.com/questions/154536/encode-decode-urls-in-c.
	// DecodeURL taken from
	// http://stackoverflow.com/questions/2673207/c-c-url-decode-library.
	std::string strEncodeURI(const std::string *value);
	std::string strEncodeURI(std::string value);
	std::string strDecodeURI(const std::string *value);
	std::string strDecodeURI(std::string value);

	// Transform b16 integer to b10 integer.
	int b16ToB10(char hex);

	// Convert two hex digits to a char and back.
	char hexToChr(std::pair<char, char> hex);
	std::pair<char, char> chrToHex(char c);

	// Push a single character to a string, where character is specified as a hex
	// pair.
	std::string *strPushHexChr(std::string *data, std::pair<char, char> hex);
	std::string strPushHexChr(std::string data, std::pair<char, char> hex);

	// Push a hex string to a string, converting the hex string to a character
	// string first.
	std::string *strPushHexStr(std::string *data, std::string *hexStr);
	std::string *strPushHexStr(std::string *data, std::string hexStr);
	std::string strPushHexStr(std::string data, std::string *hexStr);
	std::string strPushHexStr(std::string data, std::string hexStr);

	// Converts from byte string to hex string and back.
	std::string strEncodeToHex(std::string *data);
	std::string strEncodeToHex(std::string data);
	std::string strDecodeToHex(std::string *data);
	std::string strDecodeToHex(std::string data);
}

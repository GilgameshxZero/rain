// Base64 encoding and decoding.
#pragma once

#include "string.hpp"

#include <vector>

namespace Rain::String::Base64 {
	// encode/decode from https://stackoverflow.com/a/34571089/1120400. For larger
	// strings, encode/decode on-the-fly with streams instead.
	inline std::string encode(std::string const &in) {
		std::string out;

		int val = 0, valb = -6;
		for (unsigned char c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back(
					"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
						[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back(
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
					[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4) {
			out.push_back('=');
		}

		// NRVO guarantees at least move.
		return out;
	}
	inline std::string decode(std::string const &in) {
		std::string out;

		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++)
			T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] =
				i;

		unsigned int val = 0;
		int valb = -8;
		for (unsigned char c : in) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}

		// NRVO guarantees at least move.
		return out;
	}
}

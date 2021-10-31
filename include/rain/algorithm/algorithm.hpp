// Includes <algorithm> and prevents min/max ambiguity.
#pragma once

// Skip over definitions of global min/max from other sources.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>

// Override global min/max with std::min/std::max to resolve any compilation
// ambiguity.
using std::min;
using std::max;

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "../literal.hpp"

namespace Rain::Algorithm {
	// Most significant 1-bit for unsigned integral types of at most long long in
	// size. Undefined result if x = 0.
	template <typename Integer>
	inline std::size_t mostSignificant1BitIdx(Integer const x) {
#ifdef __has_builtin
#if __has_builtin(__builtin_clzll)
		return 8 * sizeof(unsigned long long) - __builtin_clzll(x);
#endif
#endif
		for (std::size_t bit = 8 * sizeof(Integer) - 1;
				 bit != std::numeric_limits<std::size_t>::max();
				 bit--) {
			if (x & (static_cast<Integer>(1) << bit)) {
				return bit;
			}
		}
		return std::numeric_limits<std::size_t>::max();
	}

	// Least significant 1-bit for unsigned integral types of at most long long in
	// size. Undefined result if x = 0.
	template <typename Integer>
	inline std::size_t leastSignificant1BitIdx(Integer const x) {
#ifdef __has_builtin
#if __has_builtin(__builtin_ctzll)
		return __builtin_ctzll(x);
#endif
#endif
		for (std::size_t bit = 0; bit != 8 * sizeof(Integer); bit++) {
			if (x & (static_cast<Integer>(1) << bit)) {
				return bit;
			}
		}
		return std::numeric_limits<std::size_t>::max();
	}

	// Count of 1-bits in unsigned integral types of at most long long in size.
	template <typename Integer>
	inline std::size_t bitPopcount(Integer const x) {
#ifdef __has_builtin
#if __has_builtin(__builtin_popcountll)
		return __builtin_popcountll(x);
#endif
#endif
		std::size_t count = 0;
		for (std::size_t bit = 0; bit != 8 * sizeof(Integer); bit++) {
			count += !!(x & (static_cast<Integer>(1) << bit));
		}
		return count;
	}

	// GCD using Euclidean algorithm.
	template <typename Integer>
	inline Integer greatestCommonDivisor(Integer x, Integer y) {
		if (x > y) {
			std::swap(x, y);
		}
		while (x != 0) {
			y %= x;
			std::swap(x, y);
		}
		return y;
	}

	// LCM. Integer type must be large enough to store product.
	template <typename Integer>
	inline Integer leastCommonMultiple(Integer const x, Integer const y) {
		return x * y / greatestCommonDivisor(x, y);
	}
}

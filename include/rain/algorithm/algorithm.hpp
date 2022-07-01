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
		return 8 * sizeof(unsigned long long) - __builtin_clzll(x) - 1;
#endif
#endif
		for (std::size_t bit{8 * sizeof(Integer) - 1};
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
		for (std::size_t bit{0}; bit != 8 * sizeof(Integer); bit++) {
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
		std::size_t count{0};
		for (std::size_t bit{0}; bit != 8 * sizeof(Integer); bit++) {
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

	// Compute the `index`-th Fibonacci matrix in $O(\log N)$ time. Helper
	// function to fibonacciNumber. `index` must be positive.
	template <typename Integer = std::size_t>
	inline std::pair<std::pair<Integer, Integer>, std::pair<Integer, Integer>>
	fibonacciMatrix(std::size_t const index) {
		if (index == 1) {
			return {{1, 1}, {1, 0}};
		}

		if (index % 2 == 0) {
			auto sub{fibonacciMatrix<Integer>(index / 2)};
			return {
				{sub.first.first * sub.first.first +
					 sub.first.second * sub.second.first,
				 sub.first.first * sub.first.second +
					 sub.first.second * sub.second.second},
				{sub.second.first * sub.first.first +
					 sub.second.second * sub.second.first,
				 sub.second.first * sub.first.second +
					 sub.second.second * sub.second.second}};
		} else {
			auto sub{fibonacciMatrix<Integer>(index - 1)};
			return {
				{sub.first.first + sub.first.second, sub.first.first},
				{sub.second.first + sub.second.second, sub.second.first}};
		}
	}

	// Compute the `index`-th Fibonacci number in $O(\log N)$ time with repeated
	// exponentiation on the 2-by-2 matrix. `index` must be non-negative.
	// fibonacciNumber(0) is defined as 0.
	template <typename Integer = std::size_t>
	inline Integer fibonacciNumber(std::size_t const index) {
		return fibonacciMatrix<Integer>(index + 1).second.second;
	}
}

#pragma once

#include <utility>

namespace Rain::Algorithm {
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

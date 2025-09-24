#pragma once

#include "algorithm.hpp"

#include <tuple>

namespace Rain::Algorithm {
	// GCD using Euclidean algorithm.
	template <typename Integer>
	inline Integer greatestCommonDivisor(Integer x, Integer y) {
		while (x != 0) {
			std::tie(y, x) = std::make_pair(x, y % x);
		}
		return y;
	}

	// GCD using extended Euclidean algorithm gives Bezout's identity
	// coefficients. `Integer` must allow integer division.
	template <typename Integer>
	inline std::tuple<Integer, Integer, Integer> greatestCommonDivisorExtended(
		Integer x,
		Integer y) {
		Integer cX{0}, cY{1}, nX{1}, nY{0}, ratio;
		while (x != 0) {
			// Relies on integer division.
			ratio = y / x;
			std::tie(y, x) = std::make_pair(x, y - ratio * x);
			std::tie(cX, nX) = std::make_pair(nX, cX - ratio * nX);
			std::tie(cY, nY) = std::make_pair(nY, cY - ratio * nY);
		}
		return {y, cX, cY};
	}

	// LCM.
	template <typename Integer>
	inline Integer leastCommonMultiple(Integer const &x, Integer const &y) {
		return x / greatestCommonDivisor(x, y) * y;
	}
}

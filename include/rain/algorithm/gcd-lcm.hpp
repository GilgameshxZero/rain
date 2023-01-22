#pragma once

#include "algorithm.hpp"

namespace Rain::Algorithm {
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

#pragma once

#include "bit-manipulators.hpp"
#include "modulus-field.hpp"

namespace Rain::Algorithm {
	// Miller-Rabin helper.
	template <typename Integer>
	bool isMaybePrimeMillerRabinInner(
		Integer const N,
		Integer const A,
		std::size_t const lsb,
		Integer const truncated) {
		ModulusField<Integer> X(N);
		X = X.build(A).power(truncated);

		if (X == 1 || X == -1) {
			return true;
		}
		for (std::size_t j{1}; j < lsb; j++) {
			X *= X;
			if (X == -1) {
				return true;
			}
		}
		return false;
	}

	// Miller-Rabin primality test in O(K ln^3 N), with failure probability upper
	// bounded by 4^-K. Can test numbers up to half the bits in Integer.
	template <typename Integer>
	bool isPrimeMillerRabin(Integer const N, std::size_t const K) {
		if (N < 4) {
			return N == 2 || N == 3;
		}

		std::size_t lsb{leastSignificant1BitIdx(N - 1)};
		Integer truncated{(N - 1) >> lsb};
		for (std::size_t i{0}; i < K; i++) {
			if (!isMaybePrimeMillerRabinInner(
						N, 2 + rand() % (N - 3), lsb, truncated)) {
				return false;
			}
		}
		return true;
	}

	// Miller-Rabin primality test, deterministic, for up to 64-bits, in O(12).
	// Bound by the same conditions on Integer as the random version.
	template <typename Integer>
	bool isPrimeMillerRabinDeterministic(Integer const N) {
		if (N < 2) {
			return false;
		}

		std::size_t lsb{leastSignificant1BitIdx(N - 1)};
		Integer truncated{(N - 1) >> lsb};
		for (Integer A : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
			if (N == A) {
				return true;
			}
			if (!isMaybePrimeMillerRabinInner(N, A, lsb, truncated)) {
				return false;
			}
		}
		return true;
	}
}

#pragma once

#include "../functional/trait.hpp"
#include "big_integer.hpp"
#include "bit_manipulators.hpp"

#include <random>

namespace Rain::Algorithm {
	// Primality tester for small ints up to 65536, or trivial
	// composite detector for larger ints.
	template<
		typename Integer,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			Functional::TraitType<Integer>::IsUnsigned::VALUE>::
			type * = nullptr>
	inline bool isTriviallyComposite(Integer N) {
		static std::uint32_t constexpr PRIMES[]{
			2,
			3,
			5,
			7,
			11,
			13,
			17,
			19,
			23,
			29,
			31,
			37,
			41,
			43,
			47,
			53,
			59,
			61,
			67,
			71,
			73,
			79,
			83,
			89,
			97,
			101,
			103,
			107,
			109,
			113,
			127,
			131,
			137,
			139,
			149,
			151,
			157,
			163,
			167,
			173,
			179,
			181,
			191,
			193,
			197,
			199,
			211,
			223,
			227,
			229,
			233,
			239,
			241,
			251};
		for (auto &i : PRIMES) {
			if (N % i == 0) {
				return true;
			}
		}
		return false;
	}

	// Miller-Rabin primality test in O(K ln^3 N), with
	// failure probability upper bounded by 4^-K. 64
	// iterations is good enough, for up to 4096 bits.
	//
	// It is required that Integer is large enough to store
	// N*N.
	//
	// We do not use ModulusField here as modulus can be an
	// expensive operation on BigInteger, so we perform it
	// manually.
	//
	// `generateRandom` must return a random Integer, uniform
	// in modulo N.
	template<
		typename Integer,
		typename Callable,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			Functional::TraitType<Integer>::IsUnsigned::VALUE>::
			type * = nullptr>
	inline bool isPrimeMillerRabin(
		Integer const &N,
		std::size_t const K,
		Callable &&generateRandom) {
		if (N < 4) {
			return N == 2 || N == 3;
		}

		std::size_t lsb{leastSignificant1BitIdx(N - 1)};
		Integer truncated((N - 1) >> lsb);

		for (std::size_t i{0}; i < K; i++) {
			Integer X(1), digit(1), multiple(generateRandom());
			// Random number in [2, N - 1].
			multiple = multiple % (N - 2) + 2;
			// Raise X to the power `truncated` under modulo N.
			for (; digit <= truncated; digit <<= 1) {
				if ((truncated & digit) != 0) {
					X = X * multiple % N;
				}
				multiple = multiple * multiple % N;
			}

			if (X == 1 || X == N - 1) {
				continue;
			}
			bool canBePrime{false};
			for (std::size_t j{1}; j < lsb; j++) {
				X = X * X % N;
				if (X == N - 1) {
					canBePrime = true;
					break;
				}
			}
			if (!canBePrime) {
				return false;
			}
		}
		return true;
	}
}

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
	// failure probability upper bounded by 4^-K.
	//
	// We do not use ModulusField here as modulus can be an
	// expensive operation on BigInteger, so we perform it
	// manually.
	template<
		typename Integer,
		typename Generator,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			Functional::TraitType<Integer>::IsUnsigned::VALUE>::
			type * = nullptr,
		decltype(std::declval<
			std::uniform_int_distribution<>>()(
			std::declval<Generator &>())) * = nullptr>
	inline bool isPrimeMillerRabin(
		Integer const &N,
		std::size_t const K,
		Generator &generator) {
		if (N < 4) {
			return N == 2 || N == 3;
		}

		// Larger integer (by 1) needed to store product before
		// performing modulus.
		using LargerInteger = BigInteger<
			MostSignificant1BitIdx<
				sizeof(Integer) * 8 - 1>::value +
				2,
			false>;
		LargerInteger largerN(N);
		std::size_t lsb{leastSignificant1BitIdx(largerN - 1)};
		LargerInteger truncated((largerN - 1) >> lsb);

		for (std::size_t i{0}; i < K; i++) {
			LargerInteger X(1), digit(1), multiple(generator);
			// Random number in [2, N - 1].
			multiple = multiple % (largerN - 2) + 2;
			// Raise X to the power `truncated` under modulo N.
			for (; digit <= truncated; digit <<= 1) {
				if ((truncated & digit) != 0) {
					X = X * multiple % largerN;
				}
				multiple = multiple * multiple % largerN;
			}

			if (X == 1 || X == largerN - 1) {
				continue;
			}
			bool canBePrime{false};
			for (std::size_t j{1}; j < lsb; j++) {
				X = X * X % largerN;
				if (X == largerN - 1) {
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

	// Default generator. 64 iterations is generally good for
	// up to 4096-bit integers.
	template<typename Integer>
	inline bool isPrimeMillerRabin(
		Integer const &N,
		std::size_t const K = 64) {
		static std::random_device randomDevice;
		static std::mt19937 generator(randomDevice());
		return isPrimeMillerRabin(N, K, generator);
	}
}

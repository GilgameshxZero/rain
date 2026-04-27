#pragma once

#include "../functional/trait.hpp"

#include <limits>

namespace Rain::Algorithm {
	// Most significant 1-bit for unsigned integral types of
	// at most long long in size. Undefined result if x = 0.
	template<
		typename Integer,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			!Functional::TraitType<Integer>::IsSigned::VALUE>::
			type * = nullptr>
	inline std::size_t mostSignificant1BitIdxImpl(
		Integer const &x) {
		for (
			std::size_t bit{8 * sizeof(Integer) - 1};
			bit != std::numeric_limits<std::size_t>::max();
			bit--) {
			if (x & (static_cast<Integer>(1) << bit)) {
				return bit;
			}
		}
		return std::numeric_limits<std::size_t>::max();
	}
	template<
		typename Integer,
		std::enable_if<std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t mostSignificant1BitIdx(
		Integer const &x) {
#ifdef __has_builtin
	#if __has_builtin(__builtin_clzll)
		return 8 * sizeof(unsigned long long) -
			__builtin_clzll(static_cast<unsigned long long>(x)) -
			1;
	#endif
#endif
		return mostSignificant1BitIdxImpl(x);
	}
	template<
		typename Integer,
		std::enable_if<!std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t mostSignificant1BitIdx(
		Integer const &x) {
		return mostSignificant1BitIdxImpl(x);
	}

	// Compile-time `mostSignificant1BitIdx`.
	template<std::size_t X>
	struct MostSignificant1BitIdx {
		static std::size_t const value{
			MostSignificant1BitIdx<(X >> 1)>::value + 1};
	};
	template<>
	struct MostSignificant1BitIdx<1> {
		static std::size_t const value{0};
	};

	// Least significant 1-bit for unsigned integral types of
	// at most long long in size. Undefined result if x = 0.
	template<
		typename Integer,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			!Functional::TraitType<Integer>::IsSigned::VALUE>::
			type * = nullptr>
	inline std::size_t leastSignificant1BitIdxImpl(
		Integer const &x) {
		for (
			std::size_t bit{0}; bit < 8 * sizeof(Integer);
			bit++) {
			if (x & (static_cast<Integer>(1) << bit)) {
				return bit;
			}
		}
		return std::numeric_limits<std::size_t>::max();
	}
	template<
		typename Integer,
		std::enable_if<std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t leastSignificant1BitIdx(
		Integer const &x) {
#ifdef __has_builtin
	#if __has_builtin(__builtin_ctzll)
		return __builtin_ctzll(
			static_cast<unsigned long long>(x));
	#endif
#endif
		return leastSignificant1BitIdxImpl(x);
	}
	template<
		typename Integer,
		std::enable_if<!std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t leastSignificant1BitIdx(
		Integer const &x) {
		return leastSignificant1BitIdxImpl(x);
	}

	// Count of 1-bits in unsigned integral types of at most
	// long long in size.
	template<
		typename Integer,
		std::enable_if<
			Functional::TraitType<Integer>::IsIntegral::VALUE &&
			!Functional::TraitType<Integer>::IsSigned::VALUE>::
			type * = nullptr>
	inline std::size_t bitPopcountImpl(Integer const &x) {
		std::size_t count{0};
		for (
			std::size_t bit{0}; bit < 8 * sizeof(Integer);
			bit++) {
			count += !!(x & (static_cast<Integer>(1) << bit));
		}
		return count;
	}
	template<
		typename Integer,
		std::enable_if<std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t bitPopcount(Integer const &x) {
#ifdef __has_builtin
	#if __has_builtin(__builtin_popcountll)
		return __builtin_popcountll(
			static_cast<unsigned long long>(x));
	#endif
#endif
		return bitPopcountImpl(x);
	}
	template<
		typename Integer,
		std::enable_if<!std::is_integral<Integer>::value>::type
			* = nullptr>
	inline std::size_t bitPopcount(Integer const &x) {
		return bitPopcountImpl(x);
	}
}

#pragma once

#include <cmath>
#include <type_traits>

namespace Rain::Algorithm {
	template <typename OutputType, typename InputType>
	inline typename std::
		enable_if<std::is_same<OutputType, long>::value, OutputType>::type
		round(InputType const &input) {
		return std::lround(input);
	}
	template <typename OutputType, typename InputType>
	inline typename std::
		enable_if<std::is_same<OutputType, long long>::value, OutputType>::type
		round(InputType const &input) {
		return std::llround(input);
	}

	// Some compilers do not have these functions in `std`, for some reason.
	template <typename OutputType, typename InputType>
	inline OutputType floor(InputType const &input) {
		return round<OutputType>(::floorl(input));
	}
	template <typename OutputType, typename InputType>
	inline OutputType ceil(InputType const &input) {
		return round<OutputType>(::ceill(input));
	}

	// `constexpr` sqrt by <https://stackoverflow.com/a/27709195>.
	template <typename T>
	constexpr T sqrtImpl(T x, T lo, T hi) {
		if (lo == hi) {
			return lo;
		}

		const T mid = (lo + hi + 1) / 2;
		if (x / mid < mid) {
			return sqrtImpl<T>(x, lo, mid - 1);
		} else {
			return sqrtImpl(x, mid, hi);
		}
	}
	template <typename T>
	constexpr T sqrt(T x) {
		return sqrtImpl<T>(x, 0, x / 2 + 1);
	}

	// Compile-time primality checker.
	template <std::size_t N, std::size_t D>
	struct IsIndivisibleByAtMost {
		static constexpr bool value =
			(N % D != 0) && IsIndivisibleByAtMost<N, D - 1>::value;
	};
	template <std::size_t N>
	struct IsIndivisibleByAtMost<N, 1> : std::true_type {};
	// `true_type` iff N is prime.
	template <std::size_t N>
	struct IsPrime : IsIndivisibleByAtMost<N, sqrt(N)> {};
	template <>
	struct IsPrime<0> : std::false_type {};
	template <>
	struct IsPrime<1> : std::false_type {};
	template <>
	struct IsPrime<2> : std::true_type {};
}

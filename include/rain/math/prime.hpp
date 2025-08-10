#pragma once

#include <type_traits>

namespace Rain::Math {
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

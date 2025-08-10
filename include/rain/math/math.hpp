#pragma once

#include "sqrt.hpp"

#include <cmath>

namespace Rain::Math {
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
}

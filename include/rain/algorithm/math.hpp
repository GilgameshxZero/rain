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

	template <typename OutputType, typename InputType>
	inline OutputType floor(InputType const &input) {
		return round<OutputType>(std::floorl(input));
	}
	template <typename OutputType, typename InputType>
	inline OutputType ceil(InputType const &input) {
		return round<OutputType>(std::ceill(input));
	}
}

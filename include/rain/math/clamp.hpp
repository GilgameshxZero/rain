#pragma once

#include "../algorithm/algorithm.hpp"
#include "../functional/trait.hpp"

namespace Rain::Math {
	// Floating-point single-argument clamp to bound away for
	// +/- INF.
	template<
		typename Value,
		std::enable_if<Functional::TypeTrait<
			Value>::IsFloatingPoint::value>::type * = nullptr>
	inline Value clamp(Value value) {
		return std::clamp(
			value,
			std::numeric_limits<Value>::lowest(),
			std::numeric_limits<Value>::max());
	}
}

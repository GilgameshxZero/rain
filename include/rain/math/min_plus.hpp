#pragma once

#include "../algorithm/algorithm.hpp"

namespace Rain::Math {
	// A min-plus type uses min instead of + and plus instead
	// of *. It is constructed with the maximum value.
	template<typename Type>
	class MinPlus {
		private:
		Type value;

		public:
		constexpr MinPlus() :
			value{std::numeric_limits<Type>::max()} {}
		constexpr MinPlus(Type const &other) : value{other} {}

		inline constexpr operator Type const &() const {
			return this->value;
		}
		inline constexpr operator Type &() {
			return this->value;
		}

		inline auto constexpr operator+(
			MinPlus<Type> const &other) const {
			return MinPlus<Type>{
				std::min(this->value, other.value)};
		}

		inline auto constexpr operator*(
			MinPlus<Type> const &other) const {
			return MinPlus<Type>{this->value + other.value};
		}
	};
}

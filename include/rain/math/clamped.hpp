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
	inline Value constexpr clamp(Value const &value) {
		return std::clamp(
			value,
			std::numeric_limits<Value>::lowest(),
			std::numeric_limits<Value>::max());
	}

	// A clamped type clamps between each operation, ensuring
	// that +/- INF is never propagated into NaNs.
	template<typename Type>
	class Clamped {
		private:
		Type value;

		public:
		constexpr Clamped() : value{} {}
		constexpr Clamped(Type const &other) :
			value{Math::clamp(other)} {}

		inline constexpr operator Type const &() const {
			return this->value;
		}
		inline constexpr operator Type &() {
			return this->value;
		}

		template<typename OtherType>
		inline auto constexpr operator+(
			OtherType const &right) const {
			return Clamped<Type>{this->value + right};
		}
		template<typename OtherType>
		inline auto constexpr operator+=(
			OtherType const &right) {
			return *this = Clamped<Type>{this->value + right};
		}
		template<typename OtherType>
		friend inline auto constexpr operator+(
			OtherType const &left,
			Clamped<Type> const &right) {
			return Clamped<Type>{left + right.value};
		}
		template<typename OtherType>
		friend inline auto constexpr operator+=(
			OtherType &left,
			Clamped<Type> const &right) {
			return left = Clamped<Type>{left + right.value};
		}
		// Explicit specialization helps with instances of
		// ambiguous operator.
		inline auto constexpr operator+(
			Clamped<Type> const &right) const {
			return Clamped<Type>{this->value + right.value};
		}
		inline auto constexpr operator+=(
			Clamped<Type> const &right) {
			return *this =
							 Clamped<Type>{this->value + right.value};
		}

		template<typename OtherType>
		inline auto constexpr operator-(
			OtherType const &right) const {
			return Clamped<Type>{this->value - right};
		}
		template<typename OtherType>
		inline auto constexpr operator-=(
			OtherType const &right) {
			return *this = Clamped<Type>{this->value - right};
		}
		template<typename OtherType>
		friend inline auto constexpr operator-(
			OtherType const &left,
			Clamped<Type> const &right) {
			return Clamped<Type>{left - right.value};
		}
		template<typename OtherType>
		friend inline auto constexpr operator-=(
			OtherType &left,
			Clamped<Type> const &right) {
			return left = Clamped<Type>{left - right.value};
		}
		inline auto constexpr operator-(
			Clamped<Type> const &right) const {
			return Clamped<Type>{this->value - right.value};
		}
		inline auto constexpr operator-=(
			Clamped<Type> const &right) {
			return *this =
							 Clamped<Type>{this->value - right.value};
		}

		template<typename OtherType>
		inline auto constexpr operator*(
			OtherType const &right) const {
			return Clamped<Type>{this->value * right};
		}
		template<typename OtherType>
		inline auto constexpr operator*=(
			OtherType const &right) const {
			return *this = Clamped<Type>{this->value * right};
		}
		template<typename OtherType>
		friend inline auto constexpr operator*(
			OtherType const &left,
			Clamped<Type> const &right) {
			return Clamped<Type>{left * right.value};
		}
		template<typename OtherType>
		friend inline auto constexpr operator*=(
			OtherType &left,
			Clamped<Type> const &right) {
			return left = Clamped<Type>{left * right.value};
		}
		inline auto constexpr operator*(
			Clamped<Type> const &right) const {
			return Clamped<Type>{this->value * right.value};
		}
		inline auto constexpr operator*=(
			Clamped<Type> const &right) {
			return *this =
							 Clamped<Type>{this->value * right.value};
		}

		template<typename OtherType>
		inline auto constexpr operator/(
			OtherType const &right) const {
			return Clamped<Type>{this->value / right};
		}
		template<typename OtherType>
		inline auto constexpr operator/=(
			OtherType const &right) {
			return *this = Clamped<Type>{this->value / right};
		}
		template<typename OtherType>
		friend inline auto constexpr operator/(
			OtherType const &left,
			Clamped<Type> const &right) {
			return Clamped<Type>{left / right.value};
		}
		template<typename OtherType>
		friend inline auto constexpr operator/=(
			OtherType &left,
			Clamped<Type> const &right) {
			return left = Clamped<Type>{left / right.value};
		}
		inline auto constexpr operator/(
			Clamped<Type> const &right) const {
			return Clamped<Type>{this->value / right.value};
		}
		inline auto constexpr operator/=(
			Clamped<Type> const &right) {
			return *this =
							 Clamped<Type>{this->value / right.value};
		}
	};
}

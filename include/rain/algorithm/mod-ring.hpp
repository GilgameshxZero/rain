// Implementation for a prime modulus ring over the integers, supporting basic
// operations add, subtract, multiply in O(1) and divide in O(ln N).
#pragma once

#include "../literal.hpp"

#include <vector>

namespace Rain::Algorithm {
	// Implementation for a prime modulus ring over the integers, supporting basic
	// operations add, subtract, multiply in O(1) and divide in O(ln N).
	//
	// Integer must be large enough to store (primeModulus - 1)^2.
	//
	// For O(1) division, cache multiplicative inverses and multiply with those.
	template <typename Integer, Integer primeModulus>
	class ModRing {
		public:
		Integer value;

		// Add primeModulus first to wrap back around in the case of "negative"
		// unsigned Integer.
		ModRing(Integer value = 0) : value((primeModulus + value) % primeModulus) {}

		// O(ln N) exponentiation.
		static ModRing<Integer, primeModulus> power(
			ModRing<Integer, primeModulus> base,
			Integer exponent) {
			if (exponent == 0) {
				return {1};
			}
			auto half = power(base, exponent / 2);
			if (exponent % 2 == 0) {
				return half * half;
			} else {
				return half * half * base;
			}
		}

		ModRing<Integer, primeModulus> operator+(Integer other) const {
			return *this + ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator+(Integer other) {
			return *this + ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator+(
			ModRing<Integer, primeModulus> other) {
			return {this->value + other.value};
		}
		ModRing<Integer, primeModulus> operator+=(
			ModRing<Integer, primeModulus> other) {
			return *this = *this + other;
		}
		ModRing<Integer, primeModulus> operator++() { return *this += 1; }
		ModRing<Integer, primeModulus> operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		ModRing<Integer, primeModulus> operator-(Integer other) const {
			return *this - ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator-(Integer other) {
			return *this - ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator-(
			ModRing<Integer, primeModulus> other) {
			return {this->value - other.value};
		}
		ModRing<Integer, primeModulus> operator-=(
			ModRing<Integer, primeModulus> other) {
			return *this = *this - other;
		}
		ModRing<Integer, primeModulus> operator--() { return *this -= 1; }
		ModRing<Integer, primeModulus> operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}
		ModRing<Integer, primeModulus> operator*(Integer other) const {
			return *this * ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator*(Integer other) {
			return *this * ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator*(
			ModRing<Integer, primeModulus> other) {
			return {this->value * other.value};
		}
		ModRing<Integer, primeModulus> operator*=(
			ModRing<Integer, primeModulus> other) {
			return *this = *this * other;
		}
		ModRing<Integer, primeModulus> operator/(Integer other) const {
			return *this / ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator/(Integer other) {
			return *this / ModRing<Integer, primeModulus>(other);
		}
		ModRing<Integer, primeModulus> operator/(
			ModRing<Integer, primeModulus> other) {
			return *this * power(other, primeModulus - 2);
		}
		ModRing<Integer, primeModulus> operator/=(
			ModRing<Integer, primeModulus> other) {
			return *this = *this / other;
		}

		bool operator==(Integer other) {
			return *this == ModRing<Integer, primeModulus>(other);
		}
		bool operator==(ModRing<Integer, primeModulus> other) {
			return this->value == other.value;
		}
	};
}

template <typename LeftInteger, typename Integer, Integer primeModulus>
inline Rain::Algorithm::ModRing<Integer, primeModulus> operator+(
	LeftInteger left,
	Rain::Algorithm::ModRing<Integer, primeModulus> right) {
	return Rain::Algorithm::ModRing<Integer, primeModulus>(left) + right;
}

template <typename LeftInteger, typename Integer, Integer primeModulus>
inline Rain::Algorithm::ModRing<Integer, primeModulus> operator-(
	LeftInteger left,
	Rain::Algorithm::ModRing<Integer, primeModulus> right) {
	return Rain::Algorithm::ModRing<Integer, primeModulus>(left) - right;
}

template <typename LeftInteger, typename Integer, Integer primeModulus>
inline Rain::Algorithm::ModRing<Integer, primeModulus> operator*(
	LeftInteger left,
	Rain::Algorithm::ModRing<Integer, primeModulus> right) {
	return Rain::Algorithm::ModRing<Integer, primeModulus>(left) * right;
}

template <typename LeftInteger, typename Integer, Integer primeModulus>
inline Rain::Algorithm::ModRing<Integer, primeModulus> operator/(
	LeftInteger left,
	Rain::Algorithm::ModRing<Integer, primeModulus> right) {
	return Rain::Algorithm::ModRing<Integer, primeModulus>(left) / right;
}

// Ease-of-use streaming operator.
template <typename Integer, Integer primeModulus>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::ModRing<Integer, primeModulus> const right) {
	return stream << right.value;
}

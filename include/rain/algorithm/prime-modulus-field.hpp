// Implementation for a prime modulus ring over the integers, supporting basic
// operations add, subtract, multiply in O(1) and divide in O(ln N).
#pragma once

#include <iostream>
#include <vector>

namespace Rain::Algorithm {
	// The Modulus interface provides one public function which returns a prime
	// modulus integer. Template specializations may not use dependent types, so
	// we must either use `integral_constant` or `enable_if` to specify the
	// this->primeModulus().
	template <auto PRIME_MODULUS>
	class PrimeModulus {
		public:
		inline auto primeModulus() const { return PRIME_MODULUS; }
	};

	// A non-compile-time prime modulus may be specified by defining a class which
	// satisfies the PrimeModulus contract, e.g. implements primeModulus.
	// Unfortunately virtual auto isn’t supported so we cannot derive primeModulus
	// easily.
	//
	// Implementation for a prime modulus ring over the integers, supporting basic
	// operations add, subtract, multiply in O(1) and divide in O(ln M). For O(1)
	// division, cache multiplicative inverses and multiply with those.
	//
	// Integer must be large enough to store (primeModulus() - 1)^2.
	template <typename Integer, typename PrimeModulus>
	class PrimeModulusField : public PrimeModulus {
		public:
		Integer value;

		// Add primeModulus() first to wrap back around in the case of "negative"
		// unsigned Integer.
		PrimeModulusField(Integer const value = 0)
				: value((this->primeModulus() + value) % this->primeModulus()) {}

		// Versions of C++ before C++17 should use static member functions intead of
		// static inline member variables. static inline
		// std::vector<PrimeModulusField<Integer, PrimeModulus>> 	&factorials() {
		// static std::vector<PrimeModulusField<Integer, PrimeModulus>> factorials;
		// return factorials;
		// }
		// static inline std::vector<PrimeModulusField<Integer, PrimeModulus>>
		// 	&invFactorials() {
		// 	static std::vector<PrimeModulusField<Integer, PrimeModulus>>
		// invFactorials; 	return invFactorials;
		// }
		static inline std::vector<PrimeModulusField<Integer, PrimeModulus>>
			factorials, invFactorials;

		// Computes the factorials modulus a prime, up to and including N, in O(N).
		// This enables the choose functions.
		static void precomputeFactorials(std::size_t const N) {
			factorials.resize(N + 1);
			invFactorials.resize(N + 1);
			factorials[0] = 1;
			for (std::size_t i{1}; i <= N; i++) {
				factorials[i] = factorials[i - 1] * i;
			}
			invFactorials[N] = 1 / factorials[N];
			for (std::size_t i{0}; i < N; i++) {
				invFactorials[N - i - 1] = invFactorials[N - i] * (N - i);
			}
		}

		// Computes the binomial coefficient (N choose K) modulus a prime, in O(1).
		// Must have called precomputeFactorials for the largest expected value of N
		// first.
		inline PrimeModulusField<Integer, PrimeModulus> choose(
			Integer const K) const {
			if (K < 0 || K > this->value) {
				return {0};
			}
			return factorials[this->value] * invFactorials[K] *
				invFactorials[this->value - K];
		}

		// O(ln N) exponentiation.
		PrimeModulusField<Integer, PrimeModulus> power(
			std::size_t const exponent) const {
			if (exponent == 0) {
				return {1};
			}
			auto half = this->power(exponent / 2);
			if (exponent % 2 == 0) {
				return half * half;
			} else {
				return half * half * *this;
			}
		}

		// Arithmetic operators.
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator+(
			OtherInteger const other) const {
			return *this + PrimeModulusField<Integer, PrimeModulus>(other);
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator+(
			OtherInteger const other) {
			return *this + PrimeModulusField<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator+(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return {this->value + other.value};
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator+=(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return *this = *this + other;
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator++() {
			return *this += 1;
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator-(
			OtherInteger const other) const {
			return *this - PrimeModulusField<Integer, PrimeModulus>(other);
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator-(
			OtherInteger const other) {
			return *this - PrimeModulusField<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator-(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return {this->value - other.value};
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator-=(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return *this = *this - other;
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator--() {
			return *this -= 1;
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator*(
			OtherInteger const other) const {
			return *this * PrimeModulusField<Integer, PrimeModulus>(other);
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator*(
			OtherInteger const other) {
			return *this * PrimeModulusField<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator*(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return {this->value * other.value};
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator*=(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return *this = *this * other;
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator/(
			OtherInteger const other) const {
			return *this / PrimeModulusField<Integer, PrimeModulus>(other);
		}
		template <typename OtherInteger>
		inline PrimeModulusField<Integer, PrimeModulus> operator/(
			OtherInteger const other) {
			return *this / PrimeModulusField<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator/(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return *this * other.power(this->primeModulus() - 2);
		}
		inline PrimeModulusField<Integer, PrimeModulus> operator/=(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return *this = *this / other;
		}

		// Equality operators.
		template <typename OtherInteger>
		inline bool operator==(OtherInteger const other) {
			return *this == PrimeModulusField<Integer, PrimeModulus>(other);
		}
		inline bool operator==(
			PrimeModulusField<Integer, PrimeModulus> const other) {
			return this->value == other.value;
		}

		// Cast operators.
		operator std::size_t() const { return this->value; }

		template <
			typename =
				std::enable_if<!std::is_same<Integer, std::size_t>::value>::type>
		operator Integer() const {
			return this->value;
		}
	};
}

template <typename OtherInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> operator+(
	OtherInteger const left,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus>(left) +
		right;
}

template <typename OtherInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> operator-(
	OtherInteger const left,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus>(left) -
		right;
}

template <typename OtherInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> operator*(
	OtherInteger const left,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus>(left) *
		right;
}

template <typename OtherInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> operator/(
	OtherInteger const left,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus>(left) /
		right;
}

// Ease-of-use streaming operators.
template <typename Integer, typename PrimeModulus>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> const right) {
	return stream << right.value;
}
template <typename Integer, typename PrimeModulus>
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Algorithm::PrimeModulusField<Integer, PrimeModulus> right) {
	stream >> right.value;
	right.value = (right.primeModulus() + right.value) % right.primeModulus();
	return stream;
}

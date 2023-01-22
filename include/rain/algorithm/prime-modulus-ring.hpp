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
	template <typename Integer, Integer PRIME_MODULUS = 0, typename = void>
	class PrimeModulus {
		public:
		virtual inline Integer primeModulus() const = 0;
	};

	template <typename Integer, Integer PRIME_MODULUS>
	class PrimeModulus<
		Integer,
		PRIME_MODULUS,
		typename std::enable_if<PRIME_MODULUS != 0>::type> {
		public:
		virtual inline Integer primeModulus() const { return PRIME_MODULUS; }
	};

	// A non-compile-time prime modulus may be specified by overriding
	// `PrimeModulus<Integer, 0>`, e.g.
	//
	// class PM : public PrimeModulus<LL> {
	// 	public:
	// 	virtual LL primeModulus() const override { return M; }
	// };

	// Implementation for a prime modulus ring over the integers, supporting basic
	// operations add, subtract, multiply in O(1) and divide in O(ln N). For O(1)
	// division, cache multiplicative inverses and multiply with those.
	//
	// Integer must be large enough to store (primeModulus() - 1)^2.
	template <typename Integer, typename PrimeModulus>
	class PrimeModulusRing : public PrimeModulus {
		public:
		Integer value;

		// Add primeModulus() first to wrap back around in the case of "negative"
		// unsigned Integer.
		PrimeModulusRing(Integer const value = 0)
				: value((this->primeModulus() + value) % this->primeModulus()) {}

		// In C++17+, these variables may be declared static inline. To preserve
		// C++11 compatibility, we access them via a function instead.
		static inline std::vector<PrimeModulusRing<Integer, PrimeModulus>>
			&factorials() {
			static std::vector<PrimeModulusRing<Integer, PrimeModulus>> factorials;
			return factorials;
		}
		static inline std::vector<PrimeModulusRing<Integer, PrimeModulus>>
			&invFactorials() {
			static std::vector<PrimeModulusRing<Integer, PrimeModulus>> invFactorials;
			return invFactorials;
		}

		// Computes the factorials modulus a prime, up to and including N, in O(N).
		// This enables the choose functions.
		static void precomputeFactorials(std::size_t const N) {
			factorials().resize(N + 1);
			invFactorials().resize(N + 1);
			factorials()[0] = 1;
			for (std::size_t i{1}; i <= N; i++) {
				factorials()[i] = factorials()[i - 1] * i;
			}
			invFactorials()[N] = 1 / factorials()[N];
			for (std::size_t i{0}; i < N; i++) {
				invFactorials()[N - i - 1] = invFactorials()[N - i] * (N - i);
			}
		}

		// Computes the binomial coefficient (N choose K) modulus a prime, in O(1).
		inline PrimeModulusRing<Integer, PrimeModulus> choose(
			Integer const K) const {
			if (K < 0 || K > this->value) {
				return {0};
			}
			return factorials()[this->value] * invFactorials()[K] *
				invFactorials()[this->value - K];
		}

		// O(ln N) exponentiation.
		PrimeModulusRing<Integer, PrimeModulus> power(
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
		inline PrimeModulusRing<Integer, PrimeModulus> operator+(
			Integer const other) const {
			return *this + PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator+(
			Integer const other) {
			return *this + PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator+(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return {this->value + other.value};
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator+=(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return *this = *this + other;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator++() {
			return *this += 1;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator-(
			Integer const other) const {
			return *this - PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator-(
			Integer const other) {
			return *this - PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator-(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return {this->value - other.value};
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator-=(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return *this = *this - other;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator--() {
			return *this -= 1;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator*(
			Integer const other) const {
			return *this * PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator*(
			Integer const other) {
			return *this * PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator*(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return {this->value * other.value};
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator*=(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return *this = *this * other;
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator/(
			Integer const other) const {
			return *this / PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator/(
			Integer const other) {
			return *this / PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator/(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return *this * other.power(this->primeModulus() - 2);
		}
		inline PrimeModulusRing<Integer, PrimeModulus> operator/=(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return *this = *this / other;
		}

		inline bool operator==(Integer const other) {
			return *this == PrimeModulusRing<Integer, PrimeModulus>(other);
		}
		inline bool operator==(
			PrimeModulusRing<Integer, PrimeModulus> const other) {
			return this->value == other.value;
		}
	};

}

template <typename LeftInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> operator+(
	LeftInteger const left,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus>(left) + right;
}

template <typename LeftInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> operator-(
	LeftInteger const left,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus>(left) - right;
}

template <typename LeftInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> operator*(
	LeftInteger const left,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus>(left) * right;
}

template <typename LeftInteger, typename Integer, typename PrimeModulus>
inline Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> operator/(
	LeftInteger const left,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	return Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus>(left) / right;
}

// Ease-of-use streaming operators.
template <typename Integer, typename PrimeModulus>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	return stream << right.value;
}
template <typename Integer, typename PrimeModulus>
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Algorithm::PrimeModulusRing<Integer, PrimeModulus> const right) {
	stream >> right.value;
	right.value = (right.primeModulus() + right.value) % right.primeModulus();
	return stream;
}

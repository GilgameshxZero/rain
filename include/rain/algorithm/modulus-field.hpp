// Implementation for a prime modulus ring over the integers, supporting basic
// operations add, subtract, multiply in O(1) and divide in O(ln N).
#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>

namespace Rain::Algorithm {
		// Implementation for a modulus field over the integers,
	// supporting basic operations add, subtract, multiply in O(1) and divide in
	// O(ln M). Division is generally only valid for prime moduli. For O(1)
	// division, cache multiplicative inverses and multiply with those.
	//
	// A runtime modulus may be specified with MODULUS 0 in the template and the appropriate constructor.
	//
	// Integer must be large enough to store (modulus() - 1)^2.
	template <typename Integer, std::size_t MODULUS = 0>
	class ModulusField {
		public:
		Integer const modulus;
		Integer value;

		// If the integer specified is signed and negative, we want to wrap it back
		// to the positives first.
		template <
			typename OtherInteger = std::size_t,
			std::size_t MODULUS_INNER = MODULUS,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		ModulusField(OtherInteger const &value = 0)
				: modulus{MODULUS},
					value(
						value < 0 ? (value % static_cast<OtherInteger>(this->modulus) +
												 static_cast<OtherInteger>(this->modulus)) %
								static_cast<OtherInteger>(this->modulus)
											: value % static_cast<OtherInteger>(this->modulus)) {}

		template <
			typename OtherInteger = std::size_t,
			std::size_t MODULUS_INNER = MODULUS,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusField(Integer const &modulus, OtherInteger const &value = 0)
				: modulus{modulus},
					value(
						value < 0 ? (value % static_cast<OtherInteger>(this->modulus) +
												 static_cast<OtherInteger>(this->modulus)) %
								static_cast<OtherInteger>(this->modulus)
											: value % static_cast<OtherInteger>(this->modulus)) {}

		// Builds a ModulusField<Integer, MODULUS> type, but with the same
		// underlying modulus value. Uses more specialized SFINAE to differentiate
		// otherwise identical signatures.
		template <
			typename OtherInteger,
			std::size_t MODULUS_INNER = MODULUS,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		static ModulusField<Integer, MODULUS> build(OtherInteger const &value) {
			return {value};
		}

		template <
			typename OtherInteger,
			std::size_t MODULUS_INNER = MODULUS,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusField<Integer, MODULUS> build(OtherInteger const &value) const {
			return {this->modulus, value};
		}

		// Assignment operators need to be overloaded as this class stores an
		// additional modulus, which implicitly deletes the default assignment
		// operator.
		ModulusField<Integer, MODULUS> &operator=(
			ModulusField<Integer, MODULUS> const &other) {
			// Runtime moduli may not actually be the same, so we need to take an
			// additional mod here.
			this->value = other.value % this->modulus;
			return *this;
		}
		template <typename OtherInteger>
		ModulusField<Integer, MODULUS> &operator=(OtherInteger const &other) {
			return *this = build(other);
		}

		// Comparison.
		template <typename OtherInteger>
		inline bool operator==(OtherInteger const &other) const {
			return *this == build(other);
		}
		template <typename OtherInteger>
		inline bool operator==(OtherInteger const &other) {
			return std::as_const(*this) == other;
		}
		inline bool operator==(ModulusField<Integer, MODULUS> const &other) const {
			// Ignores modulus comparison! Important.
			return this->value == other.value;
		}
		inline bool operator==(ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) == other;
		}
		template <typename OtherInteger>
		inline bool operator!=(OtherInteger const &other) const {
			return *this != build(other);
		}
		template <typename OtherInteger>
		inline bool operator!=(OtherInteger const &other) {
			return std::as_const(*this) != other;
		}
		inline bool operator!=(ModulusField<Integer, MODULUS> const &other) const {
			return !(*this == other);
		}
		inline bool operator!=(ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) != other;
		}
		// Other comparison operators don't make that much sense under modulus.

		// Unary.
		inline ModulusField<Integer, MODULUS> operator-() const {
			return 0 - *this;
		}
		inline ModulusField<Integer, MODULUS> operator-() {
			return -std::as_const(*this);
		}

		// Cast.
		explicit operator bool() const { return this->value != 0; }
		operator std::size_t() const {
			return static_cast<std::size_t>(this->value);
		}
		template <
			typename = typename std::enable_if<
				!std::is_same<Integer, std::size_t>::value>::type>
		operator Integer() const {
			return this->value;
		}

		// Arithmetic.
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator+(
			OtherInteger const &other) const {
			return *this + build(other);
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator+(OtherInteger const &other) {
			return std::as_const(*this) + other;
		}
		inline ModulusField<Integer, MODULUS> operator+(
			ModulusField<Integer, MODULUS> const &other) const {
			return build(this->value + other.value);
		}
		inline ModulusField<Integer, MODULUS> operator+(
			ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) + other;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> &operator+=(
			OtherInteger const &other) {
			return *this = *this + other;
		}
		inline ModulusField<Integer, MODULUS> operator++() { return *this += 1; }
		inline ModulusField<Integer, MODULUS> operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator-(
			OtherInteger const &other) const {
			return *this - build(other);
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator-(OtherInteger const &other) {
			return std::as_const(*this) - other;
		}
		inline ModulusField<Integer, MODULUS> operator-(
			ModulusField<Integer, MODULUS> const &other) const {
			return build(this->value + this->modulus - other.value);
		}
		inline ModulusField<Integer, MODULUS> operator-(
			ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) - other;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> &operator-=(
			OtherInteger const &other) {
			return *this = *this - other;
		}
		inline ModulusField<Integer, MODULUS> operator--() { return *this -= 1; }
		inline ModulusField<Integer, MODULUS> operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator*(
			OtherInteger const &other) const {
			return *this * build(other);
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator*(OtherInteger const &other) {
			return std::as_const(*this) * other;
		}
		inline ModulusField<Integer, MODULUS> operator*(
			ModulusField<Integer, MODULUS> const &other) const {
			return build(this->value * other.value);
		}
		inline ModulusField<Integer, MODULUS> operator*(
			ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) * other;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> &operator*=(
			OtherInteger const &other) {
			return *this = *this * other;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator/(
			OtherInteger const &other) const {
			return *this / build(other);
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> operator/(OtherInteger const &other) {
			return std::as_const(*this) / other;
		}
		inline ModulusField<Integer, MODULUS> operator/(
			ModulusField<Integer, MODULUS> const &other) const {
			// This is only true if this has a multiplicative inverse, which is always
			// true if the modulus is prime.
			return *this * other.power(this->modulus - 2);
		}
		inline ModulusField<Integer, MODULUS> operator/(
			ModulusField<Integer, MODULUS> const &other) {
			return std::as_const(*this) / other;
		}
		template <typename OtherInteger>
		inline ModulusField<Integer, MODULUS> &operator/=(
			OtherInteger const &other) {
			return *this = *this / other;
		}

		// The following are algorithmic functions based on the modulus field
		// properties.

		// Versions of C++ before C++17 should use static member functions intead of
		// static inline member variables. static inline
		// std::vector<ModulusField<Integer, MODULUS>> 	&factorials() {
		// static std::vector<ModulusField<Integer, MODULUS>> factorials;
		// return factorials;
		// }
		// static inline std::vector<ModulusField<Integer, MODULUS>>
		// 	&invFactorials() {
		// 	static std::vector<ModulusField<Integer, MODULUS>>
		// invFactorials; 	return invFactorials;
		// }
		static inline std::vector<ModulusField<Integer, MODULUS>> factorials,
			invFactorials;

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
		inline ModulusField<Integer, MODULUS> choose(Integer const K) const {
			if (K < 0 || K > this->value) {
				return build(0);
			}
			return factorials[this->value] * invFactorials[K] *
				invFactorials[this->value - K];
		}

		// O(ln N) exponentiation.
		template <typename OtherInteger>
		ModulusField<Integer, MODULUS> power(OtherInteger const &exponent) const {
			if (exponent == 0) {
				return build(1);
			}
			auto half = this->power(exponent / 2);
			if (exponent % 2 == 0) {
				return half * half;
			} else {
				return half * half * *this;
			}
		}
	};
}

template <typename OtherInteger, typename Integer, std::size_t MODULUS>
inline Rain::Algorithm::ModulusField<Integer, MODULUS> operator+(
	OtherInteger const &left,
	Rain::Algorithm::ModulusField<Integer, MODULUS> const &right) {
	return right.build(left) + right;
}

template <typename OtherInteger, typename Integer, std::size_t MODULUS>
inline Rain::Algorithm::ModulusField<Integer, MODULUS> operator-(
	OtherInteger const &left,
	Rain::Algorithm::ModulusField<Integer, MODULUS> const &right) {
	return right.build(left) - right;
}

template <typename OtherInteger, typename Integer, std::size_t MODULUS>
inline Rain::Algorithm::ModulusField<Integer, MODULUS> operator*(
	OtherInteger const &left,
	Rain::Algorithm::ModulusField<Integer, MODULUS> const &right) {
	return right.build(left) * right;
}

template <typename OtherInteger, typename Integer, std::size_t MODULUS>
inline Rain::Algorithm::ModulusField<Integer, MODULUS> operator/(
	OtherInteger const &left,
	Rain::Algorithm::ModulusField<Integer, MODULUS> const &right) {
	return right.build(left) / right;
}

// Ease-of-use streaming operators.
template <typename Integer, std::size_t MODULUS>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::ModulusField<Integer, MODULUS> const &right) {
	return stream << right.value;
}
template <typename Integer, std::size_t MODULUS>
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Algorithm::ModulusField<Integer, MODULUS> &right) {
	stream >> right.value;
	right.value = (right.modulus + right.value) % right.modulus;
	return stream;
}

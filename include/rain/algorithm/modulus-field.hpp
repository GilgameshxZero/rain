// Implementation for a prime modulus ring over the integers, supporting basic
// operations add, subtract, multiply in O(1) and divide in O(ln N).
#pragma once

#include "../random.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace Rain::Algorithm {
	template <
		typename = std::nullptr_t,
		typename = std::nullptr_t,
		std::size_t = 0>
	class ModulusRingBase;

	template <>
	class ModulusRingBase<std::nullptr_t, std::nullptr_t, 0> {
		public:
		// We must use our own templated SFINAE base class checker, since the one
		// provided in Functional cannot work with non-type template parameters.
		template <typename Derived, typename Underlying, std::size_t MODULUS_OUTER>
		static std::true_type isDerivedFromModulusRingImpl(
			ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const *);
		// Must include default types here in case deduction fails (which it almost
		// certainly will, and trigger SFINAE if it does, which is bad, since we
		// need this type to be defined).
		template <typename = void *, typename = void *, std::size_t = 0>
		static std::false_type isDerivedFromModulusRingImpl(...);
		template <typename TypeDerived>
		using isDerivedFromModulusRing =
			decltype(isDerivedFromModulusRingImpl(std::declval<TypeDerived *>()));
	};

	// Implementation for a modulus ring CRTP over the integers,
	// supporting basic operations add, subtract, multiply in O(1) and divide in
	// O(ln M). Division is generally only valid for prime moduli. For O(1)
	// division, cache multiplicative inverses and multiply with those.
	//
	// A runtime modulus may be specified with MODULUS 0 in the template and the
	// appropriate constructor.
	//
	// Underlying must be large enough to store (modulus() - 1)^2.
	//
	// Polymorphism CRTP is similar to the 2D CRTP in Networking, but without the
	// additional layers and complexity.
	template <typename Derived, typename Underlying, std::size_t MODULUS_OUTER>
	class ModulusRingBase {
		private:
		using TypeThis = ModulusRingBase<Derived, Underlying, MODULUS_OUTER>;
		template <typename TypeDerived>
		using isDerivedFromModulusRing =
			ModulusRingBase<>::isDerivedFromModulusRing<TypeDerived>;

		public:
		Underlying const MODULUS;
		Underlying value;

		// Construction with a raw value will directly take the value. Construction
		// with another ModulusRingBase will call the % operator on the other's
		// value, which will always stay in range of the other's ring. This value is
		// then explicitly cast into the current underlying type.
		//
		// If the integer specified is signed and negative, we want to wrap it back
		// to the positives first.
		//
		// Use () instead of {} for value to explicitly trigger narrowing casts, if
		// necessary.
		template <
			typename Integer = std::size_t,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		ModulusRingBase(Integer const &value = 0)
				: MODULUS{MODULUS_OUTER},
					value(
						value < 0 ? this->MODULUS - ((0 - value) % this->MODULUS)
											: static_cast<Underlying>(value) % this->MODULUS) {}
		template <
			typename Integer = std::size_t,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusRingBase(Underlying const &modulus, Integer const &value = 0)
				: MODULUS{modulus},
					value(
						value < 0 ? this->MODULUS - ((0 - value) % this->MODULUS)
											: static_cast<Underlying>(value) % this->MODULUS) {}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		ModulusRingBase(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other)
				: MODULUS{MODULUS_OUTER},
					value(
						other.value < 0
							? this->MODULUS - ((0 - other.value) % this->MODULUS)
							: static_cast<Underlying>(other.value) % this->MODULUS) {}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusRingBase(
			Underlying const &modulus,
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other)
				: MODULUS{modulus},
					value(
						other.value < 0
							? this->MODULUS - ((0 - other.value) % this->MODULUS)
							: static_cast<Underlying>(other.value) % this->MODULUS) {}

		// Explicit copy constructor helps avoid compiler warnings on `clang`.
		ModulusRingBase(TypeThis const &other)
				: MODULUS{other.MODULUS},
					value{
						other.value < 0
							? this->MODULUS - ((0 - other.value) % this->MODULUS)
							: static_cast<Underlying>(other.value) % this->MODULUS} {}

		// Builds a Derived type, but with the same underlying modulus value. Uses
		// more specialized SFINAE to differentiate otherwise identical signatures.
		template <
			typename Integer,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		static Derived build(Integer const &value) {
			return Derived(value);
		}
		template <
			typename Integer,
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		Derived build(Integer const &value) const {
			return {this->MODULUS, value};
		}

		// Assignment operators need to be overloaded as this class stores an
		// additional modulus, which implicitly deletes the default assignment
		// operator.
		//
		// Assignment only assigns the value. On some compilers (MSVC), it is
		// helpful to have an explicit overload for the current type.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		auto &operator=(Integer const &other) {
			return *this = build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_FIELD>
		auto &operator=(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_FIELD> const
				&other) {
			// Like in the constructor, we should force this modulus first.
			this->value = other.value % this->MODULUS;
			return *this;
		}
		auto &operator=(TypeThis const &other) {
			this->value = other.value % this->MODULUS;
			return *this;
		}

		// Casts are explicit to prevent errors. However, this may still be
		// deceptive.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		explicit operator Integer() const {
			return static_cast<Integer>(this->value);
		}

		// Comparison on integer types will first bring the integer into the ring.
		// Comparison with other modulus types will compare the value directly.
		// Some operators are generated by the compiler automatically.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator==(Integer const &other) const {
			return *this == build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator==(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			// Ignores modulus comparison!
			return this->value == other.value;
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator<(Integer const &other) const {
			return *this < build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator<(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return this->value < other.value;
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator<=(Integer const &other) const {
			return *this <= build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator<=(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return *this < other || *this == other;
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator>(Integer const &other) const {
			return *this > build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator>(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return !(*this <= other);
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator>=(Integer const &other) const {
			return *this >= build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator>=(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return *this > other || *this == other;
		}

		// Arithmetic can only be performed between ModulusRingBases with the same
		// moduli, or with raw integers. Arithmetic on raw integers first builds
		// them into this type.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator+(Integer const &other) const {
			return *this + build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator+(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return build(this->value + other.value);
		}
		template <typename Integer>
		inline auto &operator+=(Integer const &other) {
			return *this = *this + other;
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator-(Integer const &other) const {
			return *this - build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator-(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			// We must explicitly add MODULUS here because the type may be signed.
			return build(this->value + this->MODULUS - other.value);
		}
		template <typename Integer>
		inline auto &operator-=(Integer const &other) {
			return *this = *this - other;
		}
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator*(Integer const &other) const {
			return *this * build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator*(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return build(this->value * other.value);
		}
		template <typename Integer>
		inline auto &operator*=(Integer const &other) {
			return *this = *this * other;
		}
		// Modulus operates directly on modded operand value.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline auto operator%(Integer const &other) const {
			return *this % build(other);
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline auto operator%(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&other) const {
			return build(this->value % other.value);
		}
		template <typename Integer>
		inline auto &operator%=(Integer const &other) {
			return *this = *this % other;
		}

		// Unary operations are mostly shorthands for binary operations.
		inline auto operator-() const { return 0 - *this; }
		inline auto operator++() { return *this += 1; }
		inline auto operator++(int) {
			auto tmp(*this);
			*this += 1;
			return build(tmp);
		}
		inline auto operator--() { return *this -= 1; }
		inline auto operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return build(tmp);
		}

		// The following are algorithmic functions based on the modulus field
		// properties.

		// Versions of C++ before C++17 should use static member functions instead
		// of static inline member variables. static inline
		// std::vector<Derived> 	&factorials() {
		// static std::vector<Derived>
		// factorials; return factorials;
		// }
		// static inline std::vector<Derived>
		// 	&invFactorials() {
		// 	static std::vector<Derived>
		// invFactorials; 	return invFactorials;
		// }
		static inline std::vector<Derived> factorials, invFactorials;

		// Computes the factorials modulus a prime, up to and including N, in O(N).
		// This enables the choose functions.
		static inline void precomputeFactorials(std::size_t const N) {
			factorials.resize(N + 1);
			invFactorials.resize(N + 1);
			factorials[0] = 1;
			for (std::size_t i{1}; i <= N; i++) {
				factorials[i] = factorials[i - 1] * i;
			}
			invFactorials[N] = build(1) / factorials[N];
			for (std::size_t i{0}; i < N; i++) {
				invFactorials[N - i - 1] = invFactorials[N - i] * (N - i);
			}
		}

		// O(ln N) exponentiation. The exponent's value is used.
		template <
			typename Integer,
			typename std::enable_if<!isDerivedFromModulusRing<Integer>::value>::type
				* = nullptr>
		inline Derived power(Integer const &exponent) const {
			// Double base case to cover 0 but also avoid a single product sometimes.
			if (exponent == 0) {
				return build(1);
			} else if (exponent == 1) {
				return build(*this);
			}
			auto half{this->power(exponent / 2)};
			if (exponent % 2 == 0) {
				return half * half;
			} else {
				return half * half * *this;
			}
		}
		template <
			typename OtherDerived,
			typename OtherUnderlying,
			std::size_t OTHER_MODULUS_OUTER>
		inline Derived power(
			ModulusRingBase<OtherDerived, OtherUnderlying, OTHER_MODULUS_OUTER> const
				&exponent) const {
			return this->power(exponent.value);
		}
	};

	// Any reasonable modulus will suffice, since we do not define multiplicative
	// inverses in a ring.
	template <typename Underlying, std::size_t MODULUS_OUTER = 0>
	class ModulusRing : public ModulusRingBase<
												ModulusRing<Underlying, MODULUS_OUTER>,
												Underlying,
												MODULUS_OUTER> {
		private:
		using TypeThis = ModulusRing<Underlying, MODULUS_OUTER>;
		using TypeSuper = ModulusRingBase<TypeThis, Underlying, MODULUS_OUTER>;

		public:
		// Constructors must be inherited with the alias name, not the original
		// name.
		using TypeSuper::TypeSuper;

		// For some reason, we must provide specific constructors for `build`, and
		// they are not inherited.
		template <
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		ModulusRing(TypeSuper const &other) : TypeSuper(other) {}
		template <
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusRing(Underlying const &modulus, TypeSuper const &other)
				: TypeSuper(modulus, other) {}

		using TypeSuper::build;

		// `operator=` is always hidden by a dummy one in the derived class:
		// <https://stackoverflow.com/questions/12009865/operator-and-functions-that-are-not-inherited-in-c>.
		template <typename Integer>
		inline TypeThis operator=(Integer const &other) {
			return build(TypeSuper::operator=(other));
		}

		using TypeSuper::factorials;
		using TypeSuper::invFactorials;
	};

	// Must be used with a prime modulus, but this cannot be checked at
	// compile-time. If modulus is not prime, use ModulusRing instead.
	template <typename Underlying, std::size_t MODULUS_OUTER = 0>
	class ModulusField : public ModulusRingBase<
												 ModulusField<Underlying, MODULUS_OUTER>,
												 Underlying,
												 MODULUS_OUTER> {
		private:
		using TypeThis = ModulusField<Underlying, MODULUS_OUTER>;
		using TypeSuper = ModulusRingBase<TypeThis, Underlying, MODULUS_OUTER>;

		public:
		using TypeSuper::TypeSuper;

		template <
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER != 0>::type * = nullptr>
		ModulusField(TypeSuper const &other) : TypeSuper(other) {}
		template <
			std::size_t MODULUS_INNER = MODULUS_OUTER,
			typename std::enable_if<MODULUS_INNER == 0>::type * = nullptr>
		ModulusField(Underlying const &modulus, TypeSuper const &other)
				: TypeSuper(modulus, other) {}

		using TypeSuper::build;

		template <typename Integer>
		inline TypeThis operator=(Integer const &other) {
			return build(TypeSuper::operator=(other));
		}

		// Division is only correct in fields, or rings with specific operands.
		template <
			typename Integer,
			typename std::enable_if<!std::is_same<TypeThis, Integer>::value>::type * =
				nullptr>
		inline TypeThis operator/(Integer const &other) const {
			return *this / build(other);
		}
		inline TypeThis operator/(TypeThis const &other) const {
			// This is only true if this has a multiplicative inverse, which is always
			// true if the modulus is prime.
			return *this * other.power(this->MODULUS - 2);
		}
		template <typename Integer>
		inline TypeThis &operator/=(Integer const &other) {
			return *this = *this / other;
		}

		using TypeSuper::factorials;
		using TypeSuper::invFactorials;

		// Computes the binomial coefficient (N choose K) modulus a prime, in O(1).
		// Must have called precomputeFactorials for the largest expected value of N
		// first.
		inline TypeThis choose(std::size_t const K) const {
			std::size_t const N{static_cast<std::size_t>(this->value)};
			if (K < 0 || K > N) {
				return build(0);
			}
			return factorials[N] * invFactorials[K] * invFactorials[N - K];
		}
	};

	// These operators are only called if the left operand is a raw integer.
	template <
		typename Integer,
		typename std::enable_if<
			!Rain::Algorithm::ModulusRingBase<>::isDerivedFromModulusRing<
				Integer>::value>::type * = nullptr,
		typename Derived,
		typename Underlying,
		std::size_t MODULUS_OUTER>
	inline auto operator+(
		Integer const &left,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const
			&right) {
		return right.build(left) + right;
	}
	template <
		typename Integer,
		typename std::enable_if<
			!Rain::Algorithm::ModulusRingBase<>::isDerivedFromModulusRing<
				Integer>::value>::type * = nullptr,
		typename Derived,
		typename Underlying,
		std::size_t MODULUS_OUTER>
	inline auto operator-(
		Integer const &left,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const
			&right) {
		return right.build(left) - right;
	}
	template <
		typename Integer,
		typename std::enable_if<
			!Rain::Algorithm::ModulusRingBase<>::isDerivedFromModulusRing<
				Integer>::value>::type * = nullptr,
		typename Derived,
		typename Underlying,
		std::size_t MODULUS_OUTER>
	inline auto operator*(
		Integer const &left,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const
			&right) {
		return right.build(left) * right;
	}
	template <
		typename Integer,
		typename std::enable_if<
			!Rain::Algorithm::ModulusRingBase<>::isDerivedFromModulusRing<
				Integer>::value>::type * = nullptr,
		typename Derived,
		typename Underlying,
		std::size_t MODULUS_OUTER>
	inline auto operator%(
		Integer const &left,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const
			&right) {
		return right.build(left) % right;
	}
	template <
		typename Integer,
		typename std::enable_if<
			!Rain::Algorithm::ModulusRingBase<>::isDerivedFromModulusRing<
				Integer>::value>::type * = nullptr,
		typename Underlying,
		std::size_t MODULUS_OUTER>
	inline auto operator/(
		Integer const &left,
		Rain::Algorithm::ModulusField<Underlying, MODULUS_OUTER> const &right) {
		return right.build(left) / right;
	}

	// Ease-of-use streaming operators.
	template <typename Derived, typename Underlying, std::size_t MODULUS_OUTER>
	inline std::ostream &operator<<(
		std::ostream &stream,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER> const
			&right) {
		return stream << right.value;
	}
	template <typename Derived, typename Underlying, std::size_t MODULUS_OUTER>
	inline std::istream &operator>>(
		std::istream &stream,
		Rain::Algorithm::ModulusRingBase<Derived, Underlying, MODULUS_OUTER>
			&right) {
		stream >> right.value;
		right.value = (right.MODULUS + right.value) % right.MODULUS;
		return stream;
	}
}

// Hash operator for this user-defined type, which hashes the inner value (not
// the modulus). There is no way to define hash for only the base class, so we
// define it for all user-facing classes directly:
// <https://stackoverflow.com/questions/21900707/specializing-stdhash-to-derived-classes>.
namespace std {
	template <typename Underlying, std::size_t MODULUS_OUTER>
	struct hash<Rain::Algorithm::ModulusRing<Underlying, MODULUS_OUTER>> {
		size_t operator()(
			Rain::Algorithm::ModulusRing<Underlying, MODULUS_OUTER> const &value)
			const {
			return Rain::Random::SplitMixHash<decltype(value.value)>{}(value.value);
		}
	};

	template <typename Underlying, std::size_t MODULUS_OUTER>
	struct hash<Rain::Algorithm::ModulusField<Underlying, MODULUS_OUTER>> {
		size_t operator()(
			Rain::Algorithm::ModulusField<Underlying, MODULUS_OUTER> const &value)
			const {
			return Rain::Random::SplitMixHash<decltype(value.value)>{}(value.value);
		}
	};
}

// Implements cross-platform big ints.
#pragma once

#include "../literal.hpp"
#include "../random.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <utility>

namespace Rain::Algorithm {
	// Forward declaration.
	template <std::size_t LOG_BITS, bool SIGNED>
	class BigInt;
}

namespace std {
	// Limits.
	template <std::size_t LOG_BITS, bool SIGNED>
	class numeric_limits<Rain::Algorithm::BigInt<LOG_BITS, SIGNED>> {
		public:
		using ThisInt = Rain::Algorithm::BigInt<LOG_BITS, SIGNED>;

		static constexpr ThisInt min() {
			return {
				numeric_limits<typename ThisInt::SmallerIntUnsigned>::min(),
				numeric_limits<typename ThisInt::SmallerInt>::min()};
		};
		static constexpr ThisInt max() {
			return {
				numeric_limits<typename ThisInt::SmallerIntUnsigned>::max(),
				numeric_limits<typename ThisInt::SmallerInt>::max()};
		};
	};
}

namespace Rain::Algorithm {
	// Shorthands.
	template <std::size_t LOG_BITS>
	using BigIntSigned = BigInt<LOG_BITS, true>;

	template <std::size_t LOG_BITS>
	using BigIntUnsigned = BigInt<LOG_BITS, false>;

	// Maps log bits to the system type or big int type. Helps with template
	// specialization recursion.
	template <std::size_t LOG_BITS, bool SIGNED>
	class BigIntTypeMap {
		public:
		using Type = BigInt<LOG_BITS, SIGNED>;
	};

	// Template recursion base case 2**5 = 32. Requires 64-bit integer to be
	// available to support the overflow multiplication & division defined later.
	template <>
	class BigIntTypeMap<5, true> {
		public:
		using Type = std::int32_t;
	};
	template <>
	class BigIntTypeMap<5, false> {
		public:
		using Type = std::uint32_t;
	};

	// Overflow multiplication and division. Multiplication returns {low, high} of
	// the result. Division returns {low, high} of the result.
	template <std::size_t LOG_BITS, bool SIGNED>
	inline std::pair<
		typename BigIntTypeMap<LOG_BITS, false>::Type,
		typename BigIntTypeMap<LOG_BITS, SIGNED>::Type>
	bigIntMultiply(
		typename BigIntTypeMap<LOG_BITS, SIGNED>::Type const &X,
		typename BigIntTypeMap<LOG_BITS, SIGNED>::Type const &Y) {
		using ThisIntUnsigned = typename BigIntTypeMap<LOG_BITS, false>::Type;
		using ThisInt = typename BigIntTypeMap<LOG_BITS, SIGNED>::Type;
		auto [lll, llh]{bigIntMultiply<LOG_BITS - 1, false>(X.low, Y.low)};
		auto [lhl, lhh]{bigIntMultiply<LOG_BITS - 1, SIGNED>(X.low, Y.high)};
		auto [hll, hlh]{bigIntMultiply<LOG_BITS - 1, SIGNED>(X.high, Y.low)};
		auto [hhl, hhh]{bigIntMultiply<LOG_BITS - 1, SIGNED>(X.high, Y.high)};
		ThisIntUnsigned first{lll, 0};
		ThisInt second{0, hhh};
		char firstHighFlow{0}, secondLowFlow{0};
		firstHighFlow -= (llh < 0);
		first.high += llh;
		firstHighFlow +=
			(first.high >
			 std::numeric_limits<typename ThisIntUnsigned::SmallerInt>::max() - lhl);
		first.high += lhl;
		firstHighFlow +=
			(first.high >
			 std::numeric_limits<typename ThisIntUnsigned::SmallerInt>::max() - hll);
		first.high += hll;
		secondLowFlow -= (firstHighFlow < 0);
		second.low += firstHighFlow;
		secondLowFlow +=
			(lhh > 0 &&
			 second.low >
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::max() -
					 lhh) -
			(lhh < 0 &&
			 second.low <
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::min() -
					 lhh);
		second.low += lhh;
		secondLowFlow +=
			(hlh > 0 &&
			 second.low >
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::max() -
					 hlh) -
			(hlh < 0 &&
			 second.low <
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::min() -
					 hlh);
		second.low += hlh;
		secondLowFlow +=
			(hhl > 0 &&
			 second.low >
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::max() -
					 hhl) -
			(hhl < 0 &&
			 second.low <
				 std::numeric_limits<typename ThisInt::SmallerIntUnsigned>::min() -
					 hhl);
		second.low += hhl;
		second.high += secondLowFlow;
		return {first, second};
	}
	template <>
	inline std::pair<
		typename BigIntTypeMap<5, false>::Type,
		typename BigIntTypeMap<5, false>::Type>
	bigIntMultiply<5, false>(
		typename BigIntTypeMap<5, false>::Type const &X,
		typename BigIntTypeMap<5, false>::Type const &Y) {
		using ThisIntUnsigned = typename BigIntTypeMap<5, false>::Type;

		std::uint64_t result{X};
		result *= Y;
		return {
			static_cast<ThisIntUnsigned>(result),
			static_cast<ThisIntUnsigned>(result >> 32)};
	}
	template <>
	inline std::pair<
		typename BigIntTypeMap<5, false>::Type,
		typename BigIntTypeMap<5, true>::Type>
	bigIntMultiply<5, true>(
		typename BigIntTypeMap<5, true>::Type const &X,
		typename BigIntTypeMap<5, true>::Type const &Y) {
		using ThisIntUnsigned = typename BigIntTypeMap<5, false>::Type;
		using ThisInt = typename BigIntTypeMap<5, true>::Type;

		std::int64_t result{X};
		result *= Y;
		return {
			static_cast<ThisIntUnsigned>(result), static_cast<ThisInt>(result >> 32)};
	}

	template <std::size_t LOG_BITS, bool SIGNED>
	inline typename BigIntTypeMap<LOG_BITS, SIGNED>::Type bigIntDivide(
		typename BigIntTypeMap<LOG_BITS, SIGNED>::Type X,
		typename BigIntTypeMap<LOG_BITS, SIGNED>::Type Y) {
		using ThisInt = typename BigIntTypeMap<LOG_BITS, SIGNED>::Type;
		if (Y == 0) {
			throw std::runtime_error("Cannot divide by 0.");
		}

		bool negative((X < 0) ^ (Y < 0));
		if (X < 0) {
			X *= -1;
		}
		if (Y < 0) {
			Y *= -1;
		}
		if (Y == 1) {
			return negative ? -X : X;
		}

		ThisInt low{0}, high{X};
		while (low + 1 < high) {
			ThisInt mid{low + ((high - low) >> 1)};
			auto [productLow, productHigh]{bigIntMultiply<LOG_BITS, SIGNED>(mid, Y)};
			if (productHigh > 0 || productLow > X) {
				high = mid;
			} else {
				low = mid;
			}
		}
		return negative ? -low : low;
	}
	template <bool SIGNED>
	inline typename BigIntTypeMap<5, SIGNED>::Type bigIntDivide(
		typename BigIntTypeMap<5, SIGNED>::Type X,
		typename BigIntTypeMap<5, SIGNED>::Type Y) {
		return X / Y;
	}

	// Two ints of half the size form a larger int. Implements arithmetic, bit
	// shift, binary, and modulus operators, as well as comparison, i/o, and cast
	// operators. Unsigned overflow/underflow is well-defined, but not signed
	// overflow/underflow.
	template <std::size_t LOG_BITS, bool SIGNED>
	class BigInt {
		public:
		using ThisInt = BigInt<LOG_BITS, SIGNED>;
		using SmallerIntUnsigned =
			typename BigIntTypeMap<LOG_BITS - 1, false>::Type;
		using SmallerInt = typename BigIntTypeMap<LOG_BITS - 1, SIGNED>::Type;

		static std::size_t const halfBits{1_zu << (LOG_BITS - 1)};

		SmallerIntUnsigned low;
		SmallerInt high;

		template <
			typename Integer = int,
			typename std::enable_if<sizeof(Integer) * 8 <= halfBits>::type * =
				nullptr>
		BigInt(Integer const &value = 0) : low(value), high((value < 0) ? -1 : 0) {}
		template <
			typename Integer = int,
			typename std::enable_if<sizeof(Integer) * 8 == halfBits * 2>::type * =
				nullptr>
		BigInt(Integer const &value = 0)
				: low(static_cast<SmallerIntUnsigned>(value)),
					high(static_cast<SmallerInt>(value >> (1_zu << (LOG_BITS - 1)))) {}

		// Constructing with bool.
		BigInt(bool const value) : low(value ? 1 : 0), high(0) {}

		// Constructing with a larger int than storable is ill-advised and disabled.

		BigInt(SmallerIntUnsigned const &low, SmallerInt const &high = 0)
				: low(low), high(high) {}

		// Explicit copy constructor helps avoid compiler warnings on `clang`.
		BigInt(BigInt<LOG_BITS, SIGNED> const &other)
				: low{other.low}, high(other.high) {}
		template <
			bool OTHER_SIGNED,
			typename std::enable_if<OTHER_SIGNED != SIGNED>::type * = nullptr>
		BigInt(BigInt<LOG_BITS, OTHER_SIGNED> const &other)
				: low{other.low}, high(other.high) {}

		// Assignment.
		ThisInt &operator=(ThisInt const &other) {
			if (this == &other) {
				return *this;
			}
			this->low = other.low;
			this->high = other.high;
			return *this;
		}
		template <typename OtherInteger>
		ThisInt &operator=(OtherInteger const &other) {
			return *this = ThisInt(other);
		}

		// Comparison.
		template <typename OtherInteger>
		inline bool operator==(OtherInteger const &other) const {
			return *this == ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator==(OtherInteger const &other) {
			return std::as_const(*this) == other;
		}
		inline bool operator==(ThisInt const &other) const {
			return this->low == other.low && this->high == other.high;
		}
		inline bool operator==(ThisInt const &other) {
			return std::as_const(*this) == other;
		}
		template <typename OtherInteger>
		inline bool operator!=(OtherInteger const &other) const {
			return *this != ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator!=(OtherInteger const &other) {
			return std::as_const(*this) != other;
		}
		inline bool operator!=(ThisInt const &other) const {
			return !(*this == other);
		}
		inline bool operator!=(ThisInt const &other) {
			return std::as_const(*this) != other;
		}
		template <typename OtherInteger>
		inline bool operator<(OtherInteger const &other) const {
			return *this < ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator<(OtherInteger const &other) {
			return std::as_const(*this) < other;
		}
		inline bool operator<(ThisInt const &other) const {
			return this->high < other.high ||
				(this->high == other.high && this->low < other.low);
		}
		inline bool operator<(ThisInt const &other) {
			return std::as_const(*this) < other;
		}
		template <typename OtherInteger>
		inline bool operator<=(OtherInteger const &other) const {
			return *this <= ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator<=(OtherInteger const &other) {
			return std::as_const(*this) <= other;
		}
		inline bool operator<=(ThisInt const &other) const {
			return *this < other || *this == other;
		}
		inline bool operator<=(ThisInt const &other) {
			return std::as_const(*this) <= other;
		}
		template <typename OtherInteger>
		inline bool operator>(OtherInteger const &other) const {
			return *this > ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator>(OtherInteger const &other) {
			return std::as_const(*this) > other;
		}
		inline bool operator>(ThisInt const &other) const {
			return !(*this <= other);
		}
		inline bool operator>(ThisInt const &other) {
			return std::as_const(*this) > other;
		}
		template <typename OtherInteger>
		inline bool operator>=(OtherInteger const &other) const {
			return *this >= ThisInt(other);
		}
		template <typename OtherInteger>
		inline bool operator>=(OtherInteger const &other) {
			return std::as_const(*this) >= other;
		}
		inline bool operator>=(ThisInt const &other) const {
			return !(*this < other);
		}
		inline bool operator>=(ThisInt const &other) {
			return std::as_const(*this) >= other;
		}

		// Shift.
		template <typename OtherInteger>
		inline ThisInt operator>>(OtherInteger const &shift) const {
			if (shift >= static_cast<OtherInteger>(halfBits * 2)) {
				return 0;
			} else if (shift >= static_cast<OtherInteger>(halfBits)) {
				return ThisInt{static_cast<SmallerIntUnsigned>(this->high), 0} >>
					(shift - halfBits);
			}
			ThisInt tmp{this->low >> shift, this->high >> shift};
			SmallerInt lost{this->high - (tmp.high << shift)};
			return {tmp.low + (lost << (halfBits - shift)), tmp.high};
		}
		template <typename OtherInteger>
		inline ThisInt operator>>(OtherInteger const &shift) {
			return std::as_const(*this) >> shift;
		}
		template <typename OtherInteger>
		inline ThisInt &operator>>=(OtherInteger const &shift) {
			return *this = *this >> shift;
		}
		template <typename OtherInteger>
		inline ThisInt operator<<(OtherInteger const &shift) const {
			if (shift >= static_cast<OtherInteger>(halfBits * 2)) {
				return 0;
			} else if (shift >= static_cast<OtherInteger>(halfBits)) {
				return ThisInt{0, static_cast<SmallerInt>(this->low)}
				<< (shift - halfBits);
			}
			ThisInt tmp{this->low << shift, this->high << shift};
			SmallerIntUnsigned lost{this->low - (tmp.low >> shift)};
			return {
				tmp.low,
				tmp.high + static_cast<SmallerInt>(lost >> (halfBits - shift))};
		}
		template <typename OtherInteger>
		inline ThisInt operator<<(OtherInteger const &shift) {
			return std::as_const(*this) << shift;
		}
		template <typename OtherInteger>
		inline ThisInt &operator<<=(OtherInteger const &shift) {
			return *this = *this << shift;
		}

		// Binary.
		inline ThisInt operator&(ThisInt const &other) const {
			return {this->low & other.low, this->high & other.high};
		}
		inline ThisInt operator&(ThisInt const &other) {
			return std::as_const(*this) & other;
		}
		inline ThisInt operator|(ThisInt const &other) const {
			return {this->low & other.low, this->high & other.high};
		}
		inline ThisInt operator|(ThisInt const &other) {
			return std::as_const(*this) | other;
		}

		// Unary.
		inline ThisInt operator-() const { return ThisInt(0, 0) - *this; }
		inline ThisInt operator-() { return -std::as_const(*this); }

		// Cast.
		explicit operator bool() const { return *this != 0; }
		operator BigInt<LOG_BITS, !SIGNED>() const {
			return {
				this->low,
				static_cast<typename BigIntTypeMap<LOG_BITS - 1, !SIGNED>::Type>(
					this->high)};
		}
		operator typename BigIntTypeMap<5, SIGNED>::Type() const {
			if (this->high < 0) {
				auto tmp{
					std::numeric_limits<SmallerIntUnsigned>::max() - this->low + 1};
				return static_cast<typename BigIntTypeMap<5, SIGNED>::Type>(0 - tmp);
			}
			return static_cast<typename BigIntTypeMap<5, SIGNED>::Type>(this->low);
		}

		// Arithmetic.
		template <typename OtherInteger>
		inline ThisInt operator+(OtherInteger const &other) const {
			return *this + ThisInt(other);
		}
		template <typename OtherInteger>
		inline ThisInt operator+(OtherInteger const &other) {
			return *this + ThisInt(other);
		}
		inline ThisInt operator+(ThisInt const &other) const {
			// Unsigned overflow is well-defined.
			return {
				this->low + other.low,
				this->high + other.high +
					(other.low >
					 std::numeric_limits<SmallerIntUnsigned>::max() - this->low)};
		}
		inline ThisInt operator+(ThisInt const &other) {
			return std::as_const(*this) + other;
		}
		inline ThisInt &operator+=(ThisInt const &other) {
			return *this = *this + other;
		}
		inline ThisInt operator++() { return *this += 1; }
		inline ThisInt operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline ThisInt operator-(OtherInteger const &other) const {
			return *this - ThisInt(other);
		}
		template <typename OtherInteger>
		inline ThisInt operator-(OtherInteger const &other) {
			return *this - ThisInt(other);
		}
		inline ThisInt operator-(ThisInt const &other) const {
			// Unsigned overflow is well-defined.
			return {
				this->low - other.low,
				this->high - other.high - (this->low < other.low)};
		}
		inline ThisInt operator-(ThisInt const &other) {
			return std::as_const(*this) - other;
		}
		inline ThisInt &operator-=(ThisInt const &other) {
			return *this = *this - other;
		}
		inline ThisInt operator--() { return *this -= 1; }
		inline ThisInt operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}
		template <typename OtherInteger>
		inline ThisInt operator*(OtherInteger const &other) const {
			return *this * ThisInt(other);
		}
		template <typename OtherInteger>
		inline ThisInt operator*(OtherInteger const &other) {
			return *this * ThisInt(other);
		}
		inline ThisInt operator*(ThisInt const &other) const {
			auto [first, second]{bigIntMultiply<LOG_BITS, SIGNED>(*this, other)};
			if (second < 0) {
				auto tmp{
					std::numeric_limits<
						typename BigIntTypeMap<LOG_BITS, false>::Type>::max() -
					first + 1};
				return ThisInt(0, 0) - ThisInt(tmp.low, tmp.high);
			}
			return first;
		}
		inline ThisInt operator*(ThisInt const &other) {
			return std::as_const(*this) * other;
		}
		inline ThisInt &operator*=(ThisInt const &other) {
			return *this = *this * other;
		}
		template <typename OtherInteger>
		inline ThisInt operator/(OtherInteger const &other) const {
			return *this / ThisInt(other);
		}
		template <typename OtherInteger>
		inline ThisInt operator/(OtherInteger const &other) {
			return *this / ThisInt(other);
		}
		inline ThisInt operator/(ThisInt const &other) const {
			return bigIntDivide<LOG_BITS, SIGNED>(*this, other);
		}
		inline ThisInt operator/(ThisInt const &other) {
			return std::as_const(*this) / other;
		}
		inline ThisInt &operator/=(ThisInt const &other) {
			return *this = *this / other;
		}
		template <typename OtherInteger>
		inline ThisInt operator%(OtherInteger const &other) const {
			return *this % ThisInt(other);
		}
		template <typename OtherInteger>
		inline ThisInt operator%(OtherInteger const &other) {
			return *this % ThisInt(other);
		}
		inline ThisInt operator%(ThisInt const &other) const {
			auto quotient{*this / other};
			return *this - other * quotient;
		}
		inline ThisInt operator%(ThisInt const &other) {
			return std::as_const(*this) % other;
		}
		inline ThisInt &operator%=(ThisInt const &other) {
			return *this = *this % other;
		}
	};
}

template <typename OtherInteger, std::size_t LOG_BITS, bool SIGNED>
inline Rain::Algorithm::BigInt<LOG_BITS, SIGNED> operator+(
	OtherInteger const &left,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	return Rain::Algorithm::BigInt<LOG_BITS, SIGNED>(left) + right;
}
template <typename OtherInteger, std::size_t LOG_BITS, bool SIGNED>
inline Rain::Algorithm::BigInt<LOG_BITS, SIGNED> operator-(
	OtherInteger const &left,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	return Rain::Algorithm::BigInt<LOG_BITS, SIGNED>(left) - right;
}
template <typename OtherInteger, std::size_t LOG_BITS, bool SIGNED>
inline Rain::Algorithm::BigInt<LOG_BITS, SIGNED> operator*(
	OtherInteger const &left,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	return Rain::Algorithm::BigInt<LOG_BITS, SIGNED>(left) * right;
}
template <typename OtherInteger, std::size_t LOG_BITS, bool SIGNED>
inline Rain::Algorithm::BigInt<LOG_BITS, SIGNED> operator/(
	OtherInteger const &left,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	return Rain::Algorithm::BigInt<LOG_BITS, SIGNED>(left) / right;
}
template <typename OtherInteger, std::size_t LOG_BITS, bool SIGNED>
inline Rain::Algorithm::BigInt<LOG_BITS, SIGNED> operator%(
	OtherInteger const &left,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	return Rain::Algorithm::BigInt<LOG_BITS, SIGNED>(left) % right;
}

template <std::size_t LOG_BITS, bool SIGNED>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &right) {
	if (right < 0) {
		stream.put('-');
		return stream << -right;
	}
	if (right > 9) {
		stream << (right / 10);
	}
	auto first{right % 10};
	auto second{
		static_cast<typename Rain::Algorithm::BigIntTypeMap<5, SIGNED>::Type>(
			first)};
	return stream.put(second + '0');
}
template <std::size_t LOG_BITS, bool SIGNED>
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Algorithm::BigInt<LOG_BITS, SIGNED> &right) {
	std::string s;
	stream >> s;
	right = 0;
	for (char const &i : s) {
		right = right * 10 + (i - '0');
	}
	return stream;
}

// Hash operator for this user-defined type, which combines hashes of the inner
// type.
namespace std {
	template <std::size_t LOG_BITS, bool SIGNED>
	struct hash<Rain::Algorithm::BigInt<LOG_BITS, SIGNED>> {
		size_t operator()(
			Rain::Algorithm::BigInt<LOG_BITS, SIGNED> const &value) const {
			size_t result{
				Rain::Random::SplitMixHash<decltype(value.high)>{}(value.high)};
			Rain::Random::combineHash(
				result, Rain::Random::SplitMixHash<decltype(value.low)>{}(value.low));
			return result;
		}
	};
}

// Implements cross-platform big ints.
#pragma once

#include "../algorithm/bit_manipulators.hpp"
#include "../functional/trait.hpp"
#include "../literal.hpp"
#include "../random.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>

namespace Rain::Algorithm {
	// Forward declaration.
	template<std::size_t = 0, bool = false>
	class BigInteger;

	// Shorthands.
	template<std::size_t LOG_BITS>
	using BigIntegerSigned = BigInteger<LOG_BITS, true>;
	template<std::size_t LOG_BITS>
	using BigIntegerUnsigned = BigInteger<LOG_BITS, false>;
}

namespace std {
	// `std::numeric_limits`.
	template<std::size_t LOG_BITS, bool SIGNED>
	class numeric_limits<
		Rain::Algorithm::BigInteger<LOG_BITS, SIGNED>> {
		public:
		using ThisInteger =
			Rain::Algorithm::BigInteger<LOG_BITS, SIGNED>;

		static ThisInteger constexpr min() {
			return ThisInteger(
				numeric_limits<
					typename ThisInteger::SmallerInteger>::min(),
				numeric_limits<typename ThisInteger::
						SmallerIntegerUnsigned>::min());
		};
		static ThisInteger constexpr max() {
			return ThisInteger(
				numeric_limits<
					typename ThisInteger::SmallerInteger>::max(),
				numeric_limits<typename ThisInteger::
						SmallerIntegerUnsigned>::max());
		};
	};
	// We do not inherit directly because the return type is
	// wrong.
	template<bool SIGNED>
	class numeric_limits<
		Rain::Algorithm::BigInteger<5, SIGNED>> {
		public:
		using ThisInteger =
			Rain::Algorithm::BigInteger<5, SIGNED>;
		using UnderlyingInteger =
			typename ThisInteger::UnderlyingInteger;

		static ThisInteger constexpr min() {
			return ThisInteger{
				numeric_limits<UnderlyingInteger>::min()};
		};
		static ThisInteger constexpr max() {
			return ThisInteger{
				numeric_limits<UnderlyingInteger>::max()};
		};
	};
}

namespace Rain::Algorithm {
	template<
		typename IntegerFirst,
		// Selecting a single type will give the corresponding
		// BigInteger type.
		typename IntegerSecond = IntegerFirst,
		typename std::enable_if<
			Functional::TraitType<
				IntegerFirst>::IsIntegral::VALUE &&
			Functional::TraitType<IntegerSecond>::IsIntegral::
				VALUE>::type * = nullptr>
	struct BigIntegerCommon :
		std::conditional<
			(sizeof(IntegerFirst) >= sizeof(IntegerSecond)),
			BigInteger<
				Algorithm::MostSignificant1BitIdx<
					sizeof(IntegerFirst) * 8 - 1>::value +
					1 +
					(!Functional::TraitType<
						 IntegerFirst>::IsSigned::VALUE &&
						Functional::TraitType<
							IntegerSecond>::IsSigned::VALUE),
				Functional::TraitType<
					IntegerFirst>::IsSigned::VALUE ||
					Functional::TraitType<
						IntegerSecond>::IsSigned::VALUE>,
			BigInteger<
				Algorithm::MostSignificant1BitIdx<
					sizeof(IntegerSecond) * 8 - 1>::value +
					1 +
					(!Functional::TraitType<
						 IntegerSecond>::IsSigned::VALUE &&
						Functional::TraitType<
							IntegerFirst>::IsSigned::VALUE),
				Functional::TraitType<
					IntegerFirst>::IsSigned::VALUE ||
					Functional::TraitType<
						IntegerSecond>::IsSigned::VALUE>> {};

	// Template recursion base case 2**5 = 32. Requires 64-bit
	// integer to be available to support the overflow
	// multiplication & division defined later.
	//
	// Instead of just using `int32_t` and `uint32_t`, we wrap
	// them and modify select casting and shifting operations
	// to be standardized across implementations.
	//
	// Casting in-range will always preserve the value.
	// Casting out-of-range to an unsigned integer will do the
	// same as with native ints: pick the smallest value
	// equivalent modulo [size of destination]. Casting
	// out-of-range to a signed integer is implementation
	// defined on native ints, and here we pick the smallest
	// absolute value equivalent modulo [half-size of
	// destination]. The sign is preserved in this case.
	//
	// Shifting always implements arithmetic shift. That is,
	// while preserving sign, shifts always correspond to a
	// multiplication or division by `2^i`, rounding toward
	// negative infinity (as opposed to zero, or other
	// odd-only rounding schemes). Additionally, we guarantee
	// that large-enough shifts are defined behavior (and
	// eventually "zero-out"), and negative shifts are the
	// same as positive shifts in the other direction.
	//
	// Signed arithmetic overflow/underflow is further
	// well-defined to "wrap-around" within the same sign.
	// This is consistent with the cast-to-signed behavior.
	//
	// Because `static_cast` prefers constructors over cast
	// operators, we should write core logic in constructors.
	// This can be tricky since the direction is different.
	// All constructors and cast operators should be explicit,
	// except for the copy constructor.
	template<bool SIGNED>
	class BigInteger<5, SIGNED> {
		public:
		using ThisInteger = BigInteger<5, SIGNED>;
		using UnderlyingInteger = std::conditional<
			SIGNED,
			std::int32_t,
			std::uint32_t>::type;
		using OppositeSignInteger = BigInteger<5, !SIGNED>;

		UnderlyingInteger value;

		// Base case for basic helpers.
		inline bool constexpr isNegative() const {
			return SIGNED && this->value < UnderlyingInteger{};
		}

		// Copy constructor can be skipped as it is trivial.
		//
		// Exact underlying construction.
		explicit inline constexpr BigInteger(
			UnderlyingInteger other = UnderlyingInteger{0}) :
			value{other} {}
		// Bool constructor.
		explicit inline constexpr BigInteger(bool const value) :
			value{static_cast<UnderlyingInteger>(value)} {}
		// Random constructor. Requires Generator & in the
		// declval to collapse to an lvalue, which distribution
		// requires.
		template<
			typename Generator,
			std::enable_if<!Functional::TraitType<
				Generator>::IsIntegral::VALUE>::type * = nullptr,
			decltype(std::declval<
				std::uniform_int_distribution<>>()(
				std::declval<Generator &>())) * = nullptr>
		explicit inline BigInteger(Generator &generator) :
			value{
				std::uniform_int_distribution<UnderlyingInteger>(
					std::numeric_limits<UnderlyingInteger>::min(),
					std::numeric_limits<UnderlyingInteger>::max())(
					generator)} {}
		// Cast constructors.
		//
		// In-range casts are trivial and lossless.
		template<
			typename OtherInteger,
			std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value &&
				!std::is_same<UnderlyingInteger, OtherInteger>::
					value>::type * = nullptr,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<
				((sizeof(OtherInteger) == 4) &&
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE == SIGNED) ||
				((sizeof(OtherInteger) < 4) &&
					(SIGNED ||
						Functional::TraitType<OtherInteger>::IsSigned::
								VALUE == SIGNED))>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// This constructor is well-defined since `value` is
			// always in-range.
			value{static_cast<UnderlyingInteger>(other)} {}
		// Out-of-range cast constructors.
		//
		// 1. Construct signed to larger unsigned.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<
				(sizeof(OtherInteger) < 4) &&
				(!SIGNED &&
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE)>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Casting to unsigned is well-defined.
			value{static_cast<UnderlyingInteger>(other)} {}
		// 2. Construct signed to same unsigned.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<
				(sizeof(OtherInteger) == 4) &&
				(SIGNED !=
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE) &&
				!SIGNED>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Cast to unsigned is well-defined.
			value{static_cast<UnderlyingInteger>(other)} {}
		// 3. Construct unsigned to same signed.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<
				(sizeof(OtherInteger) == 4) &&
				(SIGNED !=
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE) &&
				SIGNED>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Cast to unsigned, unset most significant bit, and
			// cast again now that it is in-range.
			value{static_cast<UnderlyingInteger>(
				static_cast<std::uint32_t>(other) &
				std::uint32_t{0x7fffffff})} {}
		// 4. Construct to smaller unsigned.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<
				(sizeof(OtherInteger) > 4) && !SIGNED>::type * =
				nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Casting to unsigned is well-defined.
			value{static_cast<UnderlyingInteger>(other)} {}
		// 5. Construct to smaller signed.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr,
			std::enable_if<(sizeof(OtherInteger) > 4) && SIGNED>::
				type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) {
			// Cast to smaller unsigned int.
			auto smallerUnsigned{
				static_cast<std::uint32_t>(other)};
			// Cast smaller unsigned to smaller signed by
			// unsetting the sign bit and casting now that it is
			// in-range.
			auto smallerSigned{static_cast<std::int32_t>(
				smallerUnsigned & std::uint32_t{0x7fffffff})};
			// Restore sign if necessary.
			this->value = other < OtherInteger() ? smallerSigned ^
					std::numeric_limits<std::int32_t>::min()
																					 : smallerSigned;
		}

		// We only provide cast operators to native ints;
		// casting to BigInteger will utilize the target
		// BigInteger's constructor.
		//
		// 1. Casting to the underlying native int just unwraps.
		explicit inline operator UnderlyingInteger() const {
			return this->value;
		}
		// Casting to other native integer cannot unilaterally
		// cast to the corresponding BigInteger, because the
		// BigInteger may not exist (i.e. for an 8-byte int).
		// So, we split into multiple cases.
		//
		// 2. Casting to larger native integer.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(UnderlyingInteger) <
					sizeof(OtherInteger)>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(
				typename BigIntegerCommon<OtherInteger>::type(
					*this));
		}
		// 3. Casting to same size, unsigned native.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(UnderlyingInteger) == sizeof(OtherInteger) &&
				SIGNED !=
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE &&
				!Functional::TraitType<
					OtherInteger>::IsSigned::VALUE>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(this->value);
		}
		// 4. Casting to same size, signed native (from
		// unsigned): unset msb.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(UnderlyingInteger) == sizeof(OtherInteger) &&
				SIGNED !=
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE &&
				Functional::TraitType<
					OtherInteger>::IsSigned::VALUE>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(
				this->value &
				~(UnderlyingInteger{1}
					<< (sizeof(UnderlyingInteger) * 8 - 1)));
		}
		// 5. Casting to a smaller unsigned integer is
		// well-defined.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				!Functional::TraitType<
					OtherInteger>::IsSigned::VALUE &&
				(sizeof(UnderlyingInteger) >
					sizeof(OtherInteger))>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(this->value);
		}
		// 6. Casting to a smaller signed native integer.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				Functional::TraitType<
					OtherInteger>::IsSigned::VALUE &&
				(sizeof(UnderlyingInteger) >
					sizeof(OtherInteger))>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			using OtherIntegerUnsigned =
				typename std::make_unsigned<OtherInteger>::type;
			auto smallerUnsigned{
				static_cast<OtherIntegerUnsigned>(this->value)};
			auto smallerSigned{static_cast<OtherInteger>(
				smallerUnsigned &
				~(OtherIntegerUnsigned{1}
					<< (sizeof(OtherIntegerUnsigned) * 8 - 1)))};
			// Restore sign if necessary.
			return this->value < UnderlyingInteger{}
				? smallerSigned ^
					std::numeric_limits<OtherInteger>::min()
				: smallerSigned;
		}
		// Cast to bool is standard.
		explicit inline constexpr operator bool() const {
			return static_cast<bool>(this->value);
		}

		// Assignment operators just piggyback constructors.
		template<typename OtherInteger>
		inline ThisInteger constexpr &operator=(
			OtherInteger const &other) {
			return *this = ThisInteger(other);
		}

		// Comparison converts to common integral type first.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator==(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(this->value) ==
				CommonInteger(other);
		}
		inline bool constexpr operator==(
			ThisInteger const &other) const {
			return this->value == other.value;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator<(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) < CommonInteger(other);
		}
		inline bool constexpr operator<(
			ThisInteger const &other) const {
			return this->value < other.value;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator<=(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(this->value) <=
				CommonInteger(other);
		}
		inline bool constexpr operator<=(
			ThisInteger const &other) const {
			return this->value <= other.value;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator>(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(this->value) >
				CommonInteger(other);
		}
		inline bool constexpr operator>(
			ThisInteger const &other) const {
			return this->value > other.value;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator>=(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(this->value) >=
				CommonInteger(other);
		}
		inline bool constexpr operator>=(
			ThisInteger const &other) const {
			return this->value >= other.value;
		}

		// Bitwise operators auto-cast to common BigInteger
		// type.
		inline auto constexpr operator~() const {
			return ThisInteger(~this->value);
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator&(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) & CommonInteger(other);
		}
		inline auto constexpr operator&(
			ThisInteger const &other) const {
			return ThisInteger(this->value & other.value);
		}
		inline auto constexpr operator&=(
			ThisInteger const &other) {
			return *this = *this & other;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator|(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) | CommonInteger(other);
		}
		inline auto constexpr operator|(
			ThisInteger const &other) const {
			return ThisInteger(this->value | other.value);
		}
		inline auto constexpr operator|=(
			ThisInteger const &other) {
			return *this = *this | other;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator^(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) ^ CommonInteger(other);
		}
		inline auto constexpr operator^(
			ThisInteger const &other) const {
			return ThisInteger(this->value ^ other.value);
		}
		inline auto constexpr operator^=(
			ThisInteger const &other) {
			return *this = *this ^ other;
		}

		// Shift implements arithmetic shift. Shift can be with
		// any integral type. Very large shifts should be
		// well-defined. Shifting by a negative integer will
		// call the opposite shift operator.
		//
		// This will be undefined behavior for the minimum
		// negative operand for a signed operand, but that is
		// left to the caller.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, std::size_t>::value>::
				type * = nullptr>
		inline ThisInteger constexpr operator>>(
			OtherInteger const &other) const {
			if (other < OtherInteger()) {
				return *this << -other;
			}
			return *this >> static_cast<std::size_t>(other);
		}
		inline ThisInteger constexpr operator>>(
			std::size_t const &shift) const {
			if (shift == 0_zu) {
				return *this;
			}
			if (this->value >= UnderlyingInteger{}) {
				if (shift >= sizeof(UnderlyingInteger) * 8) {
					return ThisInteger();
				}
				return ThisInteger(this->value >> shift);
			}
			if (shift >= sizeof(UnderlyingInteger) * 8 - 1) {
				// UnderlyingInteger must be std::int32_t here,
				// since we know `value` is negative.
				return ThisInteger(std::int32_t{-1});
			}
			// Shift on negative LHS by first shifting the variant
			// with an unset sign bit, then replacing relevant
			// bits with `1`, and then resetting sign bit.
			UnderlyingInteger unsetValue{
				this->value ^
				std::numeric_limits<UnderlyingInteger>::min()};
			unsetValue >>= shift;
			std::size_t maskKeep{
				sizeof(UnderlyingInteger) * 8 - 1 - shift};
			UnderlyingInteger mask{
				(std::numeric_limits<UnderlyingInteger>::max() >>
					maskKeep)
				<< maskKeep};
			return ThisInteger(
				unsetValue ^ mask ^
				std::numeric_limits<UnderlyingInteger>::min());
		}
		template<typename Other>
		inline ThisInteger constexpr &operator>>=(
			Other const &other) {
			return *this = *this >> other;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, std::size_t>::value>::
				type * = nullptr>
		inline ThisInteger constexpr operator<<(
			OtherInteger const &other) const {
			if (other < OtherInteger()) {
				return *this >> -other;
			}
			return *this << static_cast<std::size_t>(other);
		}
		inline ThisInteger constexpr operator<<(
			std::size_t const &shift) const {
			if (shift == 0_zu) {
				return *this;
			}
			if (this->value >= UnderlyingInteger{}) {
				if (shift >= sizeof(UnderlyingInteger) * 8) {
					return ThisInteger();
				}
				return ThisInteger(this->value << shift);
			}
			if (shift >= sizeof(UnderlyingInteger) * 8 - 1) {
				return ThisInteger(
					std::numeric_limits<UnderlyingInteger>::min());
			}
			// Shift on negative LHS by unsetting the `shift + 1`
			// most significant bits (including the sign bit),
			// shifting, and resetting the sign bit.
			std::size_t maskKeep{
				sizeof(UnderlyingInteger) * 8 - 1 - shift};
			UnderlyingInteger mask{
				(std::numeric_limits<UnderlyingInteger>::max() >>
					maskKeep)
				<< maskKeep};
			UnderlyingInteger unsetValue{this->value & ~mask};
			unsetValue <<= shift;
			return ThisInteger(
				unsetValue ^
				std::numeric_limits<UnderlyingInteger>::min());
		}
		template<typename Other>
		inline ThisInteger constexpr &operator<<=(
			Other const &other) {
			return *this = *this << other;
		}

		// Arithmetic always returns the common integral type,
		// even multiplication, which may not fit.
		//
		// Furthermore, we enforce wrap-around behavior for
		// signed under/overflow. This is done with internal
		// functions `addWithOverflow`, `multiplyWithOverflow`,
		// etc., which always returns a pair [high, low]. high
		// is an overflow BigInteger with the same size as the
		// operands, and signed if either is signed, and low is
		// the result BigInteger with the same size as the
		// operands, but always unsigned.
		//
		// For add/subtract, the unsigned low may be cast into a
		// signed BigInteger as long as the msb is preserved
		// into the sign bit.
		//
		// All underlying arithmetic is performed on upcasts
		// into (u)int64_t. The upcasts are always in-range and
		// well-defined.
		//
		// The joint integer {high, low} is equivalent to the
		// true result, modulo the range of the unsigned low
		// integer (e.g., {0, 7} - {X, X} = {X, X, 0, 8} with X
		// as the maximum unsigned integer, or, for signed
		// integers and N the minimum signed, {N, 7} - {X,
		// M} = {-1, X, 0, 8}).
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<5, SIGNED || RIGHT_SIGNED>,
			BigInteger<5, false>>
			addWithOverflow(
				ThisInteger const &left,
				BigInteger<5, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<5, SIGNED || RIGHT_SIGNED>;
			using LargerInteger = std::int64_t;
			auto upLeft{static_cast<LargerInteger>(left)},
				upRight{static_cast<LargerInteger>(right)},
				upResult{upLeft + upRight};
			// Need to arithmetic shift upResult to the right.
			ResultInteger high;
			if (upResult >= LargerInteger()) {
				high = ResultInteger(
					upResult >> (sizeof(UnderlyingInteger) * 8));
			} else {
				// Unset the sign bit, shift right and cast, set the
				// sign bit.
				auto unsetUpResult{
					upResult ^
					std::numeric_limits<LargerInteger>::min()};
				unsetUpResult >>= (sizeof(UnderlyingInteger) * 8);
				high = ResultInteger(unsetUpResult) ^
					(SIGNED || RIGHT_SIGNED
							? std::numeric_limits<ResultInteger>::min()
							: ResultInteger(std::uint32_t{0x80000000}));
			}

			// Cast upResult down into unsigned BigInteger<5> will
			// truncate and is well-defined.
			BigInteger<5, false> low(upResult);
			return {high, low};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator+(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) + CommonInteger(other);
		}
		inline auto constexpr operator+(
			ThisInteger const &other) const {
			// Unset msb, cast to possible signed, set msb if it
			// was set before, and convert to target type.
			auto underlyingLow{
				ThisInteger::addWithOverflow(*this, other)
					.second.value};
			auto targetLow{static_cast<UnderlyingInteger>(
				underlyingLow & std::uint32_t(0x7fffffff))};
			if (underlyingLow & std::uint32_t{0x80000000}) {
				targetLow ^=
					(SIGNED ? std::numeric_limits<
											UnderlyingInteger>::min()
									: std::uint32_t{0x80000000});
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline auto constexpr operator+=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this + other);
		}
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<5, SIGNED || RIGHT_SIGNED>,
			BigInteger<5, false>>
			subtractWithOverflow(
				ThisInteger const &left,
				BigInteger<5, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<5, SIGNED || RIGHT_SIGNED>;
			using LargerInteger = std::int64_t;
			auto upLeft{static_cast<LargerInteger>(left)},
				upRight{static_cast<LargerInteger>(right)},
				upResult{upLeft - upRight};
			ResultInteger high;
			if (upResult >= LargerInteger()) {
				high = ResultInteger(
					upResult >> (sizeof(UnderlyingInteger) * 8));
			} else {
				auto unsetUpResult{
					upResult ^
					std::numeric_limits<LargerInteger>::min()};
				unsetUpResult >>= (sizeof(UnderlyingInteger) * 8);
				high = ResultInteger(unsetUpResult) ^
					(SIGNED || RIGHT_SIGNED
							? std::numeric_limits<ResultInteger>::min()
							: ResultInteger(std::uint32_t{0x80000000}));
			}
			BigInteger<5, false> low(upResult);
			return {high, low};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator-(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) - CommonInteger(other);
		}
		inline auto constexpr operator-(
			ThisInteger const &other) const {
			auto underlyingLow{
				ThisInteger::subtractWithOverflow(*this, other)
					.second.value};
			auto targetLow{static_cast<UnderlyingInteger>(
				underlyingLow & std::uint32_t(0x7fffffff))};
			if (underlyingLow & std::uint32_t{0x80000000}) {
				targetLow ^=
					(SIGNED ? std::numeric_limits<
											UnderlyingInteger>::min()
									: std::uint32_t{0x80000000});
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline auto constexpr operator-=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this - other);
		}
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<5, SIGNED || RIGHT_SIGNED>,
			BigInteger<5, false>>
			multiplyWithOverflow(
				ThisInteger const &left,
				BigInteger<5, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<5, SIGNED || RIGHT_SIGNED>;
			using LargerInteger = typename std::conditional<
				SIGNED || RIGHT_SIGNED,
				std::int64_t,
				std::uint64_t>::type;
			auto upLeft{static_cast<LargerInteger>(left)},
				upRight{static_cast<LargerInteger>(right)},
				upResult{upLeft * upRight};
			ResultInteger high;
			if (upResult >= LargerInteger()) {
				high = ResultInteger(
					upResult >> (sizeof(UnderlyingInteger) * 8));
			} else {
				auto unsetUpResult{
					upResult ^
					std::numeric_limits<LargerInteger>::min()};
				unsetUpResult >>= (sizeof(UnderlyingInteger) * 8);
				high = ResultInteger(unsetUpResult) ^
					(SIGNED || RIGHT_SIGNED
							? std::numeric_limits<ResultInteger>::min()
							: ResultInteger(std::uint32_t{0x80000000}));
			}
			BigInteger<5, false> low(upResult);
			return {high, low};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator*(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) * CommonInteger(other);
		}
		inline auto constexpr operator*(
			ThisInteger const &other) const {
			auto underlyingLow{
				ThisInteger::multiplyWithOverflow(*this, other)
					.second.value};
			auto targetLow{static_cast<UnderlyingInteger>(
				underlyingLow & std::uint32_t(0x7fffffff))};
			if (underlyingLow & std::uint32_t{0x80000000}) {
				targetLow ^=
					(SIGNED ? std::numeric_limits<
											UnderlyingInteger>::min()
									: std::uint32_t{0x80000000});
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline auto constexpr operator*=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this * other);
		}
		// The remainder is always the same sign as us. The
		// quotient is signed if either operand is signed.
		//
		// Returns {remainder, quotient}. Dividing by 0 returns
		// 0.
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			ThisInteger,
			BigInteger<5, SIGNED || RIGHT_SIGNED>>
			divideWithRemainder(
				ThisInteger const &left,
				BigInteger<5, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<5, SIGNED || RIGHT_SIGNED>;
			if (right == BigInteger<5, RIGHT_SIGNED>()) {
				return {ThisInteger(), ResultInteger()};
			}
			auto dividend{
				left < ThisInteger()
					? static_cast<std::uint32_t>(~left) + 1u
					: static_cast<std::uint32_t>(left)},
				divisor{
					right < BigInteger<5, RIGHT_SIGNED>()
						? static_cast<std::uint32_t>(~right) + 1u
						: static_cast<std::uint32_t>(right)};
			bool remainderNegative{left < ThisInteger()},
				quotientNegative{
					(left < ThisInteger()) !=
					(right < BigInteger<5, RIGHT_SIGNED>())};
			auto remainder{
				static_cast<std::int64_t>(dividend % divisor)},
				quotient{
					static_cast<std::int64_t>(dividend / divisor)};
			// Remainder and quotient are at most 2^31, and so can
			// be negated and cast safely, while remaining in
			// range of their target BigInteger types during
			// construction.
			return {
				remainderNegative ? ~ThisInteger(remainder - 1u)
													: ThisInteger(remainder),
				quotientNegative ? ~ResultInteger(quotient - 1u)
												 : ResultInteger(quotient)};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator/(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) / CommonInteger(other);
		}
		inline auto constexpr operator/(
			ThisInteger const &other) const {
			return ThisInteger::divideWithRemainder(*this, other)
				.second;
		}
		template<typename OtherInteger>
		inline auto constexpr operator/=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this / other);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator%(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) % CommonInteger(other);
		}
		inline auto constexpr operator%(
			ThisInteger const &other) const {
			return ThisInteger::divideWithRemainder(*this, other)
				.first;
		}
		template<typename OtherInteger>
		inline auto constexpr operator%=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this % other);
		}

		// Unary.
		inline ThisInteger constexpr operator-() const {
			return ThisInteger() - *this;
		}
		inline ThisInteger constexpr operator++() {
			return *this += 1;
		}
		inline ThisInteger constexpr operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		inline ThisInteger constexpr operator--() {
			return *this -= 1;
		}
		inline ThisInteger constexpr operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}

		// Free function binary operator overloads.
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator==(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) == CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator<(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) < CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator<=(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) <= CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator>(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) > CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator>=(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) >= CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator+(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) + CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator-(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) - CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator*(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) * CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator/(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) / CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator%(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) % CommonInteger(right);
		}

		// Stream operators.
		friend inline constexpr std::ostream &operator<<(
			std::ostream &stream,
			ThisInteger const &right) {
			return stream << right.value;
		}
		friend inline constexpr std::istream &operator>>(
			std::istream &stream,
			ThisInteger &right) {
			return stream >> right.value;
		}

		// Type traits.
		struct Trait {
			static inline bool constexpr IS_INTEGRAL{true};
			static inline bool constexpr IS_SIGNED{SIGNED};
			static inline bool constexpr IS_UNSIGNED{!SIGNED};
		};
	};

	// Two ints of half the size form a larger int. Implements
	// arithmetic, bit shift, binary, and modulus operators,
	// as well as comparison, i/o, and cast operators. All
	// behaviors are well-defined and mimic the properties of
	// the base case.
	template<std::size_t LOG_BITS, bool SIGNED>
	class BigInteger {
		public:
		using ThisInteger = BigInteger<LOG_BITS, SIGNED>;
		using SmallerInteger = BigInteger<LOG_BITS - 1, SIGNED>;
		using SmallerIntegerUnsigned =
			BigInteger<LOG_BITS - 1, false>;
		using OppositeSignInteger =
			BigInteger<LOG_BITS, !SIGNED>;

		static std::size_t const HALF_BITS{
			1_zu << (LOG_BITS - 1)};

		SmallerInteger high;
		SmallerIntegerUnsigned low;

		// Helpers.
		// Tests if this integer is negative without invoking
		// more complex functions.
		inline bool constexpr isNegative() const {
			return SIGNED && this->high.isNegative();
		}

		// Copy constructor can be skipped as it is trivial.
		//
		// Underlying constructor.
		explicit inline constexpr BigInteger(
			SmallerInteger const &high,
			SmallerIntegerUnsigned const &low) :
			high(high),
			low(low) {}
		// Default constructor, since we can't enforce
		// all-or-none default arguments in the underlying
		// constructor.
		explicit inline constexpr BigInteger() :
			high(),
			low() {}
		// Bool constructor.
		explicit inline constexpr BigInteger(bool const value) :
			high(),
			low(value) {}
		// Random constructor. Requires Generator & in the
		// declval to collapse to an lvalue, which distribution
		// requires.
		template<
			typename Generator,
			std::enable_if<!Functional::TraitType<
				Generator>::IsIntegral::VALUE>::type * = nullptr,
			decltype(std::declval<
				std::uniform_int_distribution<>>()(
				std::declval<Generator &>())) * = nullptr>
		explicit inline BigInteger(Generator &generator) :
			high(generator),
			low(generator) {}
		// Cast constructors. 2 constructors handle native ints,
		// the others handle BigInteger.
		//
		// 1. Constructing from a matching native int, unsigned.
		// Split into signed and unsigned cases.
		//
		// Since LOG_BITS = 5 is the base case, we only need to
		// implement this for LOG_BITS >= 6. Since integers
		// larger than 64 bits are not guaranteed, we only
		// implement this for LOG_BITS <= 6.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				LOG_BITS == 6 &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) ==
					sizeof(OtherInteger) &&
				SIGNED ==
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE &&
				!SIGNED>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Right shift is well-defined for unsigned ints.
			high(static_cast<SmallerInteger>(other >> 32)),
			// Casting to unsigned smaller is well-defined.
			low(static_cast<SmallerIntegerUnsigned>(other)) {}
		// 2. Constructing from a matching native int, signed.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				LOG_BITS == 6 &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) ==
					sizeof(OtherInteger) &&
				SIGNED ==
					Functional::TraitType<
						OtherInteger>::IsSigned::VALUE &&
				SIGNED>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// Casting to unsigned smaller is well-defined.
			low(static_cast<SmallerIntegerUnsigned>(other)) {
			auto unsignedOther{static_cast<std::uint64_t>(other)};
			auto downUnsignedOther{
				static_cast<std::uint32_t>(unsignedOther >> 32)};
			auto downSignedOther{static_cast<std::int32_t>(
				downUnsignedOther & std::uint32_t{0x7fffffff})};
			// Restore sign bit if msb was set.
			if (downUnsignedOther & std::uint32_t{0x80000000}) {
				downSignedOther ^=
					std::numeric_limits<std::int32_t>::min();
			}
			this->high = SmallerInteger(downSignedOther);
		}
		// 3. Constructing from a non-matching native int first
		// constructs the BigInteger corresponding to the native
		// int, then constructs from that BigInteger.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerIntegerUnsigned) +
							sizeof(SmallerInteger) !=
						sizeof(OtherInteger) ||
					SIGNED !=
						Functional::TraitType<OtherInteger>::IsSigned::
							VALUE)>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			BigInteger(
				typename BigIntegerCommon<OtherInteger>::type(
					other)) {}
		// 4. Constructing to the same size, opposite sign is
		// trivial by recursing on high.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) ==
					sizeof(OtherInteger) &&
				SIGNED !=
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			high(other.high),
			low(other.low) {}
		// 5. Constructing to a larger `BigInteger` of the same
		// sign recurses down.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) >
					sizeof(OtherInteger)) &&
				SIGNED ==
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			// If `other.isNegative()`, casting to an unsigned
			// int will wrap it back around to positive. But
			// since `high` will be `-1`, the joint value is
			// correct. This is well-defined.
			high(
				other.isNegative() ? SmallerInteger(-1)
													 : SmallerInteger()),
			low(other) {}
		// 6. Constructing to a smaller `BigInteger` of the same
		// sign just recurses to `low`. Even if currently
		// signed, the operation is well-defined (case 4).
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) <
					sizeof(OtherInteger)) &&
				SIGNED ==
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			BigInteger(other.low) {}
		// 7. Constructing to a different BigInteger of a
		// different sign will first cast to the resized
		// BigInteger of the same sign, then cast the sign.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(SmallerIntegerUnsigned) +
						sizeof(SmallerInteger) !=
					sizeof(OtherInteger) &&
				SIGNED !=
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE>::type * = nullptr>
		explicit inline constexpr BigInteger(
			OtherInteger const &other) :
			BigInteger(
				typename BigInteger::OppositeSignInteger(other)) {}

		// Cast operators.
		//
		// 1. Casting losslessly to a same-size, same-signed
		// native int.
		//
		// Since LOG_BITS = 5 is the base case, we only need to
		// implement this for LOG_BITS >= 6. Since integers
		// larger than 64 bits are not guaranteed, we only
		// implement this for LOG_BITS <= 6.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				LOG_BITS == 6 &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				sizeof(SmallerInteger) +
						sizeof(SmallerIntegerUnsigned) ==
					sizeof(OtherInteger) &&
				SIGNED ==
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			// Unwrap high/low.
			auto nativeHigh{static_cast<
				typename SmallerInteger::UnderlyingInteger>(
				this->high)};
			auto nativeLow{static_cast<
				typename SmallerIntegerUnsigned::UnderlyingInteger>(
				this->low)};
			// Cast up high/low. Always in-range. low is upcast to
			// the possibly signed variant (it is in-range) for
			// easy AND.
			auto upHigh{static_cast<OtherInteger>(nativeHigh)};
			auto upLow{static_cast<OtherInteger>(nativeLow)};
			// Shift and AND.
			return (upHigh << ThisInteger::HALF_BITS) | upLow;
		}
		// 2. Casting to same or larger native int which is not
		// case 1.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerInteger) +
							sizeof(SmallerIntegerUnsigned) <
						sizeof(OtherInteger) ||
					(sizeof(SmallerInteger) +
								sizeof(SmallerIntegerUnsigned) ==
							sizeof(OtherInteger) &&
						SIGNED !=
							Functional::TraitType<OtherInteger>::
								IsSigned::VALUE))>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(
				typename BigIntegerCommon<OtherInteger>::type(
					*this));
		}
		// 3. Casting to smaller unsigned int.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerInteger) +
							sizeof(SmallerIntegerUnsigned) >
						sizeof(OtherInteger) &&
					!Functional::TraitType<OtherInteger>::IsSigned::
						VALUE)>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			return static_cast<OtherInteger>(this->low);
		}
		// 4. Casting to smaller signed native int.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE &&
				(sizeof(SmallerInteger) +
							sizeof(SmallerIntegerUnsigned) >
						sizeof(OtherInteger) &&
					Functional::TraitType<OtherInteger>::IsSigned::
						VALUE)>::type * = nullptr>
		explicit inline constexpr
			operator OtherInteger() const {
			using OtherIntegerUnsigned =
				typename std::make_unsigned<OtherInteger>::type;
			auto smallerUnsigned{
				static_cast<OtherIntegerUnsigned>(this->low)};
			auto smallerSigned{static_cast<OtherInteger>(
				smallerUnsigned &
				~(OtherIntegerUnsigned{1}
					<< (sizeof(OtherIntegerUnsigned) * 8 - 1)))};
			// Restore sign if necessary.
			return this->high < SmallerInteger{} ? smallerSigned ^
					std::numeric_limits<OtherInteger>::min()
																					 : smallerSigned;
		}
		// Recursively casting to bool avoids comparators, which
		// we conceptually define after the casters.
		explicit inline constexpr operator bool() const {
			return static_cast<bool>(this->high) ||
				static_cast<bool>(this->low);
		}

		// Assignment operators just piggyback constructors.
		template<typename OtherInteger>
		inline ThisInteger constexpr &operator=(
			OtherInteger const &other) {
			return *this = ThisInteger(other);
		}

		// Comparison. Implementation mirrors base case.
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator==(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) == CommonInteger(other);
		}
		inline bool constexpr operator==(
			ThisInteger const &other) const {
			return this->high == other.high &&
				this->low == other.low;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator<(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) < CommonInteger(other);
		}
		inline bool constexpr operator<(
			ThisInteger const &other) const {
			return this->high < other.high ||
				(this->high == other.high && this->low < other.low);
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator<=(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) <= CommonInteger(other);
		}
		inline bool constexpr operator<=(
			ThisInteger const &other) const {
			return this->high < other.high ||
				(this->high == other.high &&
					this->low <= other.low);
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator>(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) > CommonInteger(other);
		}
		inline bool constexpr operator>(
			ThisInteger const &other) const {
			return this->high > other.high ||
				(this->high == other.high && this->low > other.low);
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline bool constexpr operator>=(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) >= CommonInteger(other);
		}
		inline bool constexpr operator>=(
			ThisInteger const &other) const {
			return this->high > other.high ||
				(this->high == other.high &&
					this->low >= other.low);
		}

		// Bitwise.
		inline auto constexpr operator~() const {
			return ThisInteger(~this->high, ~this->low);
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator&(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) & CommonInteger(other);
		}
		inline auto constexpr operator&(
			ThisInteger const &other) const {
			return ThisInteger(
				this->high & other.high, this->low & other.low);
		}
		inline auto constexpr operator&=(
			ThisInteger const &other) {
			return *this = *this & other;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator|(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) | CommonInteger(other);
		}
		inline auto constexpr operator|(
			ThisInteger const &other) const {
			return ThisInteger(
				this->high | other.high, this->low | other.low);
		}
		inline auto constexpr operator|=(
			ThisInteger const &other) {
			return *this = *this | other;
		}
		template<
			typename OtherInteger,
			std::enable_if<
				Functional::TraitType<
					OtherInteger>::IsIntegral::VALUE &&
				!std::is_same<OtherInteger, ThisInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator^(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) ^ CommonInteger(other);
		}
		inline auto constexpr operator^(
			ThisInteger const &other) const {
			return ThisInteger(
				this->high ^ other.high, this->low ^ other.low);
		}
		inline auto constexpr operator^=(
			ThisInteger const &other) {
			return *this = *this ^ other;
		}

		// Shift.
		//
		// OtherInteger must be capable of storing the bit-size
		// of the current BigInteger.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr>
		inline ThisInteger constexpr operator>>(
			OtherInteger const &other) const {
			if (other == OtherInteger()) {
				return *this;
			}
			if (other < OtherInteger()) {
				// Write like this to avoid MSVC C4146.
				return *this << (OtherInteger() - other);
			}
			// "Zero" out if more than bit-width of ThisInteger.
			if (
				static_cast<OtherInteger>(
					ThisInteger::HALF_BITS * 2 - SIGNED) <= other) {
				return this->isNegative() ? ThisInteger(-1)
																	: ThisInteger();
			}
			// Directly shift low/high if more than half the bit
			// width.
			if (
				static_cast<OtherInteger>(ThisInteger::HALF_BITS) <=
				other) {
				auto convertedHigh{
					SmallerIntegerUnsigned(this->high)};
				return ThisInteger(
								 SmallerInteger(
									 this->high < SmallerInteger() ? -1 : 0),
								 convertedHigh) >>
					(other -
						static_cast<OtherInteger>(
							ThisInteger::HALF_BITS));
			}
			ThisInteger result(
				this->high >> other, this->low >> other);
			// Unset the sign bit with `& ~min`, then cast to
			// unsigned now that it is in-range.
			auto carry{static_cast<SmallerIntegerUnsigned>(
				this->high &
				~std::numeric_limits<SmallerInteger>::min())};
			carry <<=
				static_cast<OtherInteger>(ThisInteger::HALF_BITS) -
				other;
			result.low ^= carry;
			return result;
		}
		template<typename Other>
		inline ThisInteger constexpr &operator>>=(
			Other const &other) {
			return *this = *this >> other;
		}
		// OtherInteger must be capable of storing the bit-size
		// of the current BigInteger.
		template<
			typename OtherInteger,
			std::enable_if<Functional::TraitType<
				OtherInteger>::IsIntegral::VALUE>::type * = nullptr>
		inline ThisInteger constexpr operator<<(
			OtherInteger const &other) const {
			if (other == OtherInteger()) {
				return *this;
			}
			if (other < OtherInteger()) {
				return *this >> (OtherInteger() - other);
			}
			// "Zero" out if more than bit-width of ThisInteger.
			if (
				static_cast<OtherInteger>(
					ThisInteger::HALF_BITS * 2 - SIGNED) <= other) {
				return this->isNegative()
					? std::numeric_limits<ThisInteger>::min()
					: ThisInteger();
			}
			// Directly shift low/high if more than half the bit
			// width.
			if (
				static_cast<OtherInteger>(ThisInteger::HALF_BITS) <=
				other) {
				auto convertedLow{SmallerInteger(this->low)};
				if (this->high < SmallerInteger()) {
					convertedLow ^=
						std::numeric_limits<SmallerInteger>::min();
				}
				return ThisInteger(
								 convertedLow, SmallerIntegerUnsigned())
					<< (other -
							 static_cast<OtherInteger>(
								 ThisInteger::HALF_BITS));
			}
			ThisInteger result(
				this->high << other, this->low << other);
			// Guaranteed to be in-range as long as `other` is
			// positive.
			auto carry{static_cast<SmallerInteger>(
				this->low >>
				(static_cast<OtherInteger>(ThisInteger::HALF_BITS) -
					other))};
			result.high ^= carry;
			return result;
		}
		template<typename Other>
		inline ThisInteger constexpr &operator<<=(
			Other const &other) {
			return *this = *this << other;
		}

		// Arithmetic.
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>,
			BigInteger<LOG_BITS, false>>
			addWithOverflow(
				ThisInteger const &left,
				BigInteger<LOG_BITS, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>;
			auto r1{SmallerInteger::addWithOverflow(
				left.high, right.high)};
			auto r2{SmallerIntegerUnsigned::addWithOverflow(
				left.low, right.low)};
			auto r3{SmallerIntegerUnsigned::addWithOverflow(
				r1.second, r2.first)};
			// The cast is lossless because the r3 overflow is
			// small.
			auto r4{
				ResultInteger::SmallerInteger::addWithOverflow(
					r1.first, SmallerInteger(r3.first))};
			return {
				ResultInteger(r4.first, r4.second),
				BigInteger<LOG_BITS, false>(r3.second, r2.second)};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator+(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) + CommonInteger(other);
		}
		inline auto constexpr operator+(
			ThisInteger const &other) const {
			// If unsigned, return immediately.
			auto unsignedLow{
				ThisInteger::addWithOverflow(*this, other).second};
			if (!SIGNED) {
				// This is always a copy construct.
				return ThisInteger(unsignedLow);
			}
			// Otherwise, just like base case: unset msb, cast to
			// signed, set msb if it was set before, and convert
			// to target type.
			auto targetLow{
				static_cast<ThisInteger>((unsignedLow << 1) >> 1)};
			if (
				unsignedLow >
				(std::numeric_limits<
					 BigInteger<LOG_BITS, false>>::max() >>
					1)) {
				targetLow ^=
					std::numeric_limits<ThisInteger>::min();
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline auto constexpr operator+=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this + other);
		}
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>,
			BigInteger<LOG_BITS, false>>
			subtractWithOverflow(
				ThisInteger const &left,
				BigInteger<LOG_BITS, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>;
			auto r1{SmallerInteger::subtractWithOverflow(
				left.high, right.high)};
			auto r2{SmallerIntegerUnsigned::subtractWithOverflow(
				left.low, right.low)};
			auto r3{SmallerIntegerUnsigned::addWithOverflow(
				r1.second, r2.first)};
			// If r2 was wrapped, then r31 needs to be
			// decremented.
			if (
				r2.first ==
				std::numeric_limits<
					SmallerIntegerUnsigned>::max()) {
				r3.first = SmallerIntegerUnsigned(
					r3.first == SmallerIntegerUnsigned() ? -1 : 0);
			} else {
				r3.first = SmallerIntegerUnsigned(r3.first);
			}
			auto r4{
				ResultInteger::SmallerInteger::addWithOverflow(
					r1.first, r3.first)};
			// TODO: Decrement can be simpler because the domain
			// of values is small.
			// If r1 was wrapped, r41 needs to be decremented.
			if (
				r1.first ==
				std::numeric_limits<SmallerInteger>::max()) {
				r4.first += SmallerInteger(-1);
			}
			// If r3 was wrapped, r41 needs to be decremented.
			if (
				r3.first ==
				std::numeric_limits<
					SmallerIntegerUnsigned>::max()) {
				r4.first += SmallerInteger(-1);
			}
			return {
				ResultInteger(r4.first, r4.second),
				BigInteger<LOG_BITS, false>(r3.second, r2.second)};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline ThisInteger constexpr operator-(
			OtherInteger const &other) const {
			return *this - ThisInteger(other);
		}
		inline ThisInteger constexpr operator-(
			ThisInteger const &other) const {
			// If unsigned, return immediately.
			auto unsignedLow{
				ThisInteger::subtractWithOverflow(*this, other)
					.second};
			if (!SIGNED) {
				// This is always a copy construct.
				return ThisInteger(unsignedLow);
			}
			// Otherwise, just like base case: unset msb, cast to
			// signed, set msb if it was set before, and convert
			// to target type.
			auto targetLow{
				static_cast<ThisInteger>((unsignedLow << 1) >> 1)};
			if (
				unsignedLow >
				(std::numeric_limits<
					 BigInteger<LOG_BITS, false>>::max() >>
					1)) {
				targetLow ^=
					std::numeric_limits<ThisInteger>::min();
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline ThisInteger constexpr &operator-=(
			OtherInteger const &other) {
			return *this = *this - other;
		}
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>,
			BigInteger<LOG_BITS, false>>
			multiplyWithOverflow(
				ThisInteger const &left,
				BigInteger<LOG_BITS, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>;
			auto r1{SmallerInteger::multiplyWithOverflow(
				left.high, right.high)};
			auto r2{SmallerIntegerUnsigned::multiplyWithOverflow(
				left.low, right.high)};
			auto r3{SmallerInteger::multiplyWithOverflow(
				left.high, right.low)};
			auto r4{SmallerIntegerUnsigned::multiplyWithOverflow(
				left.low, right.low)};

			auto r5{SmallerIntegerUnsigned::addWithOverflow(
				r2.second, r3.second)};
			auto r6{SmallerIntegerUnsigned::addWithOverflow(
				r5.second, r4.first)};

			// Result: {r11, r12 + r21 + r31 + r51 + r61, r62,
			// r42}. r12 is unsigned. r51 and r61 are both small
			// and unsigned.
			auto r7{SmallerIntegerUnsigned::addWithOverflow(
				r1.second, r2.first)};
			auto r8{SmallerIntegerUnsigned::addWithOverflow(
				r7.second, r3.first)};
			auto r9{SmallerIntegerUnsigned::addWithOverflow(
				r8.second, r5.first + r6.first)};

			// Result: {r11 + r71 + r81 + r91, r92, r62, r42}.
			// r71, r81, r91 are all close to 0. r71 is
			// RIGHT_SIGNED. r11, r81, and r91 are result-signed.
			//
			// Mathematically, there is no further overflow.
			return {
				ResultInteger(
					r1.first +
						static_cast<ResultInteger::SmallerInteger>(
							r7.first) +
						r8.first + r9.first,
					r9.second),
				BigInteger<LOG_BITS, false>(r6.second, r4.second)};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator*(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) * CommonInteger(other);
		}
		inline auto constexpr operator*(
			ThisInteger const &other) const {
			// If unsigned, return immediately.
			auto unsignedLow{
				ThisInteger::multiplyWithOverflow(*this, other)
					.second};
			if (!SIGNED) {
				// This is always a copy construct.
				return ThisInteger(unsignedLow);
			}
			// Otherwise, just like base case: unset msb, cast to
			// signed, set msb if it was set before, and convert
			// to target type.
			auto targetLow{
				static_cast<ThisInteger>((unsignedLow << 1) >> 1)};
			if (
				unsignedLow >
				(std::numeric_limits<
					 BigInteger<LOG_BITS, false>>::max() >>
					1)) {
				targetLow ^=
					std::numeric_limits<ThisInteger>::min();
			}
			return ThisInteger(targetLow);
		}
		template<typename OtherInteger>
		inline auto constexpr operator*=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this * other);
		}
		template<bool RIGHT_SIGNED>
		static inline constexpr std::pair<
			ThisInteger,
			BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>>
			divideWithRemainder(
				ThisInteger const &left,
				BigInteger<LOG_BITS, RIGHT_SIGNED> const &right) {
			using ResultInteger =
				BigInteger<LOG_BITS, SIGNED || RIGHT_SIGNED>;
			if (right == BigInteger<LOG_BITS, RIGHT_SIGNED>()) {
				return {ThisInteger(), ResultInteger()};
			}
			// Shift up divisor until it is about to exceed
			// dividend. Then, subtract it from dividend while
			// shifting downwards, collating the total count
			// subtracted. Detect this condition by comparing the
			// difference between the shifted divisor and
			// dividend; if it is less than the shifted divisor,
			// then one more shift will exceed the dividend.
			//
			// Both divisor and dividend are negated to positive
			// before beginning. They are stored in an unsigned
			// type, so overflow will not be a problem.
			auto dividend{
				left < ThisInteger()
					? BigInteger<LOG_BITS, false>(~left) + 1u
					: BigInteger<LOG_BITS, false>(left)},
				divisor{
					right < BigInteger<LOG_BITS, RIGHT_SIGNED>()
						? BigInteger<LOG_BITS, false>(~right) + 1u
						: BigInteger<LOG_BITS, false>(right)};
			bool remainderNegative{left < ThisInteger()},
				quotientNegative{
					(left < ThisInteger()) !=
					(right < BigInteger<LOG_BITS, RIGHT_SIGNED>())};
			if (divisor > dividend) {
				return {left, ResultInteger()};
			}
			std::size_t shifts{1};
			for (; dividend - divisor >= divisor; shifts++) {
				divisor <<= 1;
			}
			ResultInteger quotient;
			for (; shifts > 0; shifts--) {
				quotient <<= 1;
				if (dividend >= divisor) {
					dividend -= divisor;
					quotient += 1u;
				}
				divisor >>= 1;
			}
			// -dividend, -quotient can both fit in their
			// respective signed types.
			return {
				remainderNegative ? ~ThisInteger(dividend - 1u)
													: ThisInteger(dividend),
				quotientNegative ? ~ResultInteger(quotient - 1u)
												 : ResultInteger(quotient)};
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator/(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) / CommonInteger(other);
		}
		inline auto constexpr operator/(
			ThisInteger const &other) const {
			return ThisInteger::divideWithRemainder(*this, other)
				.second;
		}
		template<typename OtherInteger>
		inline auto constexpr operator/=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this / other);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!std::is_same<ThisInteger, OtherInteger>::value>::
				type * = nullptr>
		inline auto constexpr operator%(
			OtherInteger const &other) const {
			using CommonInteger = typename BigIntegerCommon<
				ThisInteger,
				OtherInteger>::type;
			return CommonInteger(*this) % CommonInteger(other);
		}
		inline auto constexpr operator%(
			ThisInteger const &other) const {
			return ThisInteger::divideWithRemainder(*this, other)
				.first;
		}
		template<typename OtherInteger>
		inline auto constexpr operator%=(
			OtherInteger const &other) {
			return *this =
							 static_cast<ThisInteger>(*this % other);
		}

		// Unary.
		inline ThisInteger constexpr operator-() const {
			return ThisInteger() - *this;
		}
		inline ThisInteger constexpr operator++() {
			return *this += 1;
		}
		inline ThisInteger constexpr operator++(int) {
			auto tmp(*this);
			*this += 1;
			return tmp;
		}
		inline ThisInteger constexpr operator--() {
			return *this -= 1;
		}
		inline ThisInteger constexpr operator--(int) {
			auto tmp(*this);
			*this -= 1;
			return tmp;
		}

		// Free function binary operator overloads.
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator==(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) == CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator<(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) < CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator<=(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) <= CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator>(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) > CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator>=(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) >= CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator+(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) + CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator-(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) - CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator*(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) * CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr &operator/(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) / CommonInteger(right);
		}
		template<
			typename OtherInteger,
			typename std::enable_if<
				!Functional::TraitTypeTemplateAuto<BigInteger>::
					IsBaseOfTemplate<OtherInteger>::VALUE>::type * =
				nullptr>
		friend inline ThisInteger constexpr operator%(
			OtherInteger const &left,
			ThisInteger const &right) {
			using CommonInteger = typename BigIntegerCommon<
				OtherInteger,
				ThisInteger>::type;
			return CommonInteger(left) % CommonInteger(right);
		}

		// Stream operators.
		friend inline std::ostream constexpr &operator<<(
			std::ostream &stream,
			ThisInteger const &right) {
			if (right < ThisInteger()) {
				stream.put('-');
				return stream
					<< ThisInteger::OppositeSignInteger(~right) + 1u;
			}
			// Similar to division algorithm, but we need to
			// reserve an additional 10x size, not just 2x.
			using LargerInteger = BigInteger<LOG_BITS + 1, false>;
			LargerInteger remainder(right);
			std::vector<LargerInteger> digits;
			digits.push_back(LargerInteger(1));
			while (remainder >= digits.back()) {
				digits.push_back(digits.back() * 10u);
			}
			digits.pop_back();

			for (; !digits.empty(); digits.pop_back()) {
				char digit{'0'};
				for (; remainder >= digits.back(); digit++) {
					remainder -= digits.back();
				}
				stream.put(digit);
			}
			return stream;
		}
		friend inline std::istream constexpr &operator>>(
			std::istream &stream,
			ThisInteger &right) {
			std::string s;
			stream >> s;
			right = 0;
			for (char const &i : s) {
				right = right * 10 + (i - '0');
			}
			return stream;
		}

		// Type traits.
		struct Trait {
			static inline bool constexpr IS_INTEGRAL{true};
			static inline bool constexpr IS_SIGNED{SIGNED};
			static inline bool constexpr IS_UNSIGNED{!SIGNED};
		};
	};
}

namespace std {
	template<bool SIGNED>
	struct hash<Rain::Algorithm::BigInteger<5, SIGNED>> {
		size_t operator()(
			Rain::Algorithm::BigInteger<5, SIGNED> const &value)
			const {
			return hash<typename Rain::Algorithm::
					BigInteger<5, SIGNED>::UnderlyingInteger>{}(
				value.value);
		}
	};
	// Hash operator for this user-defined type, which
	// combines hashes of the inner type.
	template<std::size_t LOG_BITS, bool SIGNED>
	struct hash<
		Rain::Algorithm::BigInteger<LOG_BITS, SIGNED>> {
		size_t operator()(
			Rain::Algorithm::BigInteger<LOG_BITS, SIGNED> const
				&value) const {
			size_t result{
				Rain::Random::SplitMixHash<decltype(value.high)>{}(
					value.high)};
			Rain::Random::combineHash(
				result,
				Rain::Random::SplitMixHash<decltype(value.low)>{}(
					value.low));
			return result;
		}
	};
}

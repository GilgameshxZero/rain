#pragma once

#include "algorithm.hpp"

#include <climits>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>

namespace Rain::Algorithm {
	// Flexible (and fast) big integer type (henceforth called
	// `BIF`) with an underlying std::vector instead of
	// recursion.
	class BigIntegerFlexUnsigned {
		public:
		using S = std::size_t;
		using T = std::uint_fast64_t;
		using BIFU = BigIntegerFlexUnsigned;

		// Stored with low bits at the beginning.
		std::vector<T> value;

		// Zeros are always trimmed off the high bits.
		inline void constexpr trim() {
			while (value.size() > 1 && value.back() == 0) {
				value.pop_back();
			}
		}

		// Constructor.
		template<typename U = T>
		inline constexpr BigIntegerFlexUnsigned(
			U const &value = 0) :
			value({static_cast<T>(value)}) {}

		// Cast.
		explicit inline constexpr operator bool() const {
			return static_cast<bool>(value[0]);
		}
		explicit inline operator std::string() const {
			std::stringstream ss;
			ss << *this;
			return ss.str();
		}

		// Comparator.
		inline auto constexpr operator==(
			BIFU const &other) const {
			if (value.size() != other.value.size()) {
				return false;
			}
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				if (value[i] != other.value[i]) {
					return false;
				}
			}
			return true;
		}
		inline auto constexpr operator<(
			BIFU const &other) const {
			if (value.size() < other.value.size()) {
				return true;
			} else if (value.size() > other.value.size()) {
				return false;
			}
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				if (value[i] < other.value[i]) {
					return true;
				} else if (value[i] > other.value[i]) {
					return false;
				}
			}
			return false;
		}
		inline auto constexpr operator<=(
			BIFU const &other) const {
			if (value.size() < other.value.size()) {
				return true;
			} else if (value.size() > other.value.size()) {
				return false;
			}
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				if (value[i] < other.value[i]) {
					return true;
				} else if (value[i] > other.value[i]) {
					return false;
				}
			}
			return true;
		}
		inline auto constexpr operator>(
			BIFU const &other) const {
			if (value.size() < other.value.size()) {
				return false;
			} else if (value.size() > other.value.size()) {
				return true;
			}
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				if (value[i] < other.value[i]) {
					return false;
				} else if (value[i] > other.value[i]) {
					return true;
				}
			}
			return false;
		}
		inline auto constexpr operator>=(
			BIFU const &other) const {
			if (value.size() < other.value.size()) {
				return false;
			} else if (value.size() > other.value.size()) {
				return true;
			}
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				if (value[i] < other.value[i]) {
					return false;
				} else if (value[i] > other.value[i]) {
					return true;
				}
			}
			return true;
		}

		// Bitwise.
		inline auto constexpr operator~() const {
			BIFU r;
			r.value.resize(value.size());
			for (auto i{r.value.size() - 1}; i != SIZE_MAX; --i) {
				r.value[i] = ~value[i];
			}
			r.trim();
			return r;
		}
		inline auto constexpr operator&(
			BIFU const &other) const {
			BIFU r;
			r.value.resize(min(value.size(), other.value.size()));
			for (auto i{r.value.size() - 1}; i != SIZE_MAX; --i) {
				r.value[i] = value[i] & other.value[i];
			}
			r.trim();
			return r;
		}
		inline auto constexpr operator&=(BIFU const &other) {
			return *this = *this & other;
		}
		inline auto constexpr operator|(
			BIFU const &other) const {
			BIFU const *smaller{this}, *larger{&other};
			if (value.size() > other.value.size()) {
				std::swap(smaller, larger);
			}
			BIFU r;
			r.value.resize(larger->value.size());
			for (
				auto i{smaller->value.size() - 1}; i != SIZE_MAX;
				--i) {
				r.value[i] = value[i] | other.value[i];
			}
			for (
				auto i{r.value.size() - 1};
				i >= smaller->value.size();
				--i) {
				r.value[i] = larger->value[i];
			}
			r.trim();
			return r;
		}
		inline auto constexpr operator|=(BIFU const &other) {
			return *this = *this | other;
		}
		inline auto constexpr operator^(
			BIFU const &other) const {
			BIFU const *smaller{this}, *larger{&other};
			if (value.size() > other.value.size()) {
				std::swap(smaller, larger);
			}
			BIFU r;
			r.value.resize(larger->value.size());
			for (
				auto i{smaller->value.size() - 1}; i != SIZE_MAX;
				--i) {
				r.value[i] = value[i] ^ other.value[i];
			}
			for (
				auto i{r.value.size() - 1};
				i >= smaller->value.size();
				--i) {
				r.value[i] = larger->value[i];
			}
			r.trim();
			return r;
		}
		inline auto constexpr operator^=(BIFU const &other) {
			return *this = *this ^ other;
		}

		// Shift. Since size is flexible, left shift never
		// truncates.
		inline auto constexpr operator>>(S shift) const {
			if (shift >= 64 * value.size()) {
				return BIFU();
			}
			if (shift >= 64) {
				BIFU r;
				auto push{shift / 64};
				r.value.resize(value.size() - push);
				for (auto i{value.size() - 1}; i >= push; --i) {
					r.value[i - push] = value[i];
				}
				return r >> (shift % 64);
			}
			BIFU r[2];
			r[0].value.resize(value.size());
			r[1].value.resize(value.size() - 1);
			r[0].value[0] = value[0] >> shift;
			for (auto i{value.size() - 1}; i > 0; --i) {
				r[0].value[i] = value[i] >> shift;
				r[1].value[i - 1] = value[i] << (64 - shift);
			}
			if (r[1].value.empty()) {
				r[1].value.push_back(0);
			}
			return r[0] ^ r[1];
		}
		inline auto constexpr operator>>=(S shift) {
			return *this = *this >> shift;
		}
		inline auto constexpr operator<<(S shift) const {
			if (shift >= 64) {
				BIFU r;
				auto push{(shift + 63) / 64};
				r.value.resize(value.size() + push);
				for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
					r.value[i + push] = value[i];
				}
				return r >> (64 - shift % 64);
			}
			BIFU r[2];
			r[0].value.resize(value.size());
			r[1].value.resize(value.size() + 1);
			for (auto i{value.size() - 1}; i != SIZE_MAX; --i) {
				r[0].value[i] = value[i] << shift;
				r[1].value[i + 1] = value[i] >> (64 - shift);
			}
			return r[0] ^ r[1];
		}
		inline auto constexpr operator<<=(S shift) {
			return *this = *this << shift;
		}

		// Arithmetic.
		inline auto constexpr operator+(
			BIFU const &other) const {
			BIFU const *smaller{this}, *larger{&other};
			if (value.size() > other.value.size()) {
				std::swap(smaller, larger);
			}
			BIFU r;
			r.value.resize(larger->value.size());
			T overflow{0}, next;
			for (S i{0}; i < smaller->value.size(); ++i) {
				r.value[i] = value[i] + other.value[i];
				next = r.value[i] < value[i];
				r.value[i] += overflow;
				next += r.value[i] < overflow;
				overflow = next;
			}
			for (
				auto i{smaller->value.size()};
				i < larger->value.size();
				++i) {
				r.value[i] = larger->value[i] + overflow;
				overflow = r.value[i] < overflow;
			}
			if (overflow) {
				r.value.push_back(overflow);
			}
			return r;
		}
		inline auto constexpr operator+=(BIFU const &other) {
			return *this = *this + other;
		}
		// Assume this is bigger than other, otherwise UB.
		inline auto constexpr operator-(
			BIFU const &other) const {
			BIFU r;
			r.value.resize(value.size());
			T underflow{0}, next;
			for (S i{0}; i < value.size(); ++i) {
				r.value[i] = value[i] -
					(i < other.value.size() ? other.value[i] : 0);
				next = r.value[i] > value[i];
				r.value[i] -= underflow;
				next += underflow > 0 && r.value[i] == ULLONG_MAX;
				underflow = next;
			}
			r.trim();
			return r;
		}
		inline auto constexpr operator-=(BIFU const &other) {
			return *this = *this - other;
		}
		// Iterate through all pairs, and split multiplication
		// between the high/low half of each uint64.
		inline auto constexpr operator*(
			BIFU const &other) const {
			BIFU r, o;
			// Final T not necessary but makes inner loop easier.
			r.value.resize(value.size() + other.value.size() + 1);
			o.value.resize(r.value.size() + 1);
			for (S i{0}; i < value.size(); ++i) {
				for (S j{0}; j < other.value.size(); ++j) {
					T lh{value[i] >> 32}, ll{value[i] << 32 >> 32},
						rh{other.value[j] >> 32},
						rl{other.value[j] << 32 >> 32};
					T xhh{lh * rh}, xhl{lh * rl}, xlh{ll * rh},
						xll{ll * rl};
					T zh{xhh}, zl{xll};
					T yh1{xhl >> 32}, yh2{xlh >> 32}, yl1{xhl << 32},
						yl2{xlh << 32};
					// Need to add yh* to zh, and yl* to zl.
					zl += yl1;
					o.value[i + j + 1] += zl < yl1;
					zl += yl2;
					o.value[i + j + 1] += zl < yl2;
					zh += yh1;
					o.value[i + j + 2] += zh < yh1;
					zh += yh2;
					o.value[i + j + 2] += zh < yh2;
					// Add to the right position in r.
					r.value[i + j] += zl;
					o.value[i + j + 1] += r.value[i + j] < zl;
					r.value[i + j + 1] += zh;
					o.value[i + j + 2] += r.value[i + j + 1] < zh;
				}
			}
			r.trim();
			o.trim();
			return r + o;
		}
		inline auto constexpr operator*=(BIFU const &other) {
			return *this = *this * other;
		}
		// Returns {remainder, quotient}.
		inline std::
			pair<BIFU, BIFU> constexpr divideWithRemainder(
				BIFU const &other) const {
			if (other > *this) {
				return {*this, 0};
			}
			auto dividend{*this}, divisor{other};
			S shifts{1};
			for (; dividend - divisor >= divisor; shifts++) {
				divisor <<= 1;
			}
			BIFU quotient;
			for (; shifts > 0; shifts--) {
				quotient <<= 1;
				if (dividend >= divisor) {
					dividend -= divisor;
					quotient += 1u;
				}
				divisor >>= 1;
			}
			return {dividend, quotient};
		}
		inline auto constexpr operator/(
			BIFU const &other) const {
			return divideWithRemainder(other).second;
		}
		inline auto constexpr operator/=(BIFU const &other) {
			return *this = *this / other;
		}
		inline auto constexpr operator%(
			BIFU const &other) const {
			return divideWithRemainder(other).first;
			return BIFU();
		}
		inline auto constexpr operator%=(BIFU const &other) {
			return *this = *this % other;
		}

		// Unary.
		inline auto constexpr operator-() const {
			return BIFU() - *this;
		}
		inline auto constexpr operator++() {
			return *this += BIFU(1);
		}
		inline auto constexpr operator++(int) {
			auto tmp(*this);
			*this += BIFU(1);
			return tmp;
		}
		inline auto constexpr operator--() {
			return *this -= BIFU(1);
		}
		inline auto constexpr operator--(int) {
			auto tmp(*this);
			*this -= BIFU(1);
			return tmp;
		}

		// Stream. << operator is used before this so return
		// type cannot be `auto`.
		friend inline std::ostream &operator<<(
			std::ostream &stream,
			BIFU const &other) {
			BIFU remainder(other);
			std::vector<BIFU> digits;
			digits.push_back(1);
			while (remainder >= digits.back()) {
				digits.push_back(digits.back() * 10);
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
		friend inline auto &operator>>(
			std::istream &stream,
			BIFU &other) {
			std::string s;
			stream >> s;
			other = 0;
			for (char const &i : s) {
				other = other * 10 + (i - '0');
			}
			return stream;
		}

		// Traits.
		struct Trait {
			static inline bool constexpr IS_INTEGRAL{true};
			static inline bool constexpr IS_SIGNED{false};
			static inline bool constexpr IS_UNSIGNED{true};
		};
	};
}

#pragma once

#include "../algorithm/bit-manipulators.hpp"
#include "../error/exception.hpp"
#include "../literal.hpp"
#include "../platform.hpp"

#include <array>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <utility>

namespace Rain::Math {
	template <typename = std::nullptr_t, std::size_t = 0>
	class Tensor;

	template <>
	class Tensor<std::nullptr_t, 0> {
		template <class, std::size_t>
		friend class Tensor;

		public:
		enum Error { NONE, SIZES_MISMATCH };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept { return "Rain::Math::Tensor"; }
			std::string message(int error) const noexcept {
				switch (error) {
					case Error::NONE:
						return "None.";
					case Error::SIZES_MISMATCH:
						return "Tensor(s) are not of compatible size for operation.";
					default:
						return "Generic.";
				}
			}
		};
		using Exception = Rain::Error::Exception<Error, ErrorCategory>;

		class Range {
			public:
			// `stop`: defaults will be replaced with current `stop` during `slice`.
			// `size`: actual underlying size.
			std::size_t start{std::numeric_limits<std::size_t>::max()},
				stop{std::numeric_limits<std::size_t>::max()},
				step{std::numeric_limits<std::size_t>::max()};
		};

		// Standard policy for Tensor products. This forms a ring.
		template <typename Left, typename Right>
		class PlusMultProductPolicy {
			public:
			static decltype(std::declval<Left>() * std::declval<Right>())
				const DEFAULT_RESULT{0};

			static constexpr inline auto sum(Left const &left, Right const &right) {
				return left + right;
			}
			static constexpr inline auto product(
				Left const &left,
				Right const &right) {
				return left * right;
			}
		};

		// Policy for Tensor products where + is replaced with min and * is replaced
		// with +. This form a semi-ring.
		template <typename Left, typename Right>
		class MinPlusProductPolicy {
			public:
			static decltype(std::declval<Left>() + std::declval<Right>())
				const DEFAULT_RESULT{std::numeric_limits<
					decltype(std::declval<Left>() + std::declval<Right>())>::max()};

			static constexpr inline auto sum(Left const &left, Right const &right) {
				return std::min(left, right);
			}
			static constexpr inline auto product(
				Left const &left,
				Right const &right) {
				return left + right;
			}
		};

		private:
		// Perform an operation over all indices, or over the first few indices.
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER + 1> const &result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				callable(
					result[i], std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
		// An explicit overload for `applyOver` which does nothing, which may be
		// called from `product` if one matrix is completely contracted.
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER> const &result,
			auto &&...others) {
			callable(result, std::forward<decltype(others)>(others)...);
		}
		// `applyOver` which reduces ORDER.
		template <
			std::size_t REMAINING_ORDER,
			typename ResultValue,
			std::size_t RESULT_ORDER,
			bool isReducing = (REMAINING_ORDER < RESULT_ORDER),
			typename std::enable_if<isReducing>::type * = nullptr>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, RESULT_ORDER> const &result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				Tensor<>::applyOver<REMAINING_ORDER>(
					std::forward<decltype(callable)>(callable),
					result[i],
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}

		// L-value versions of `applyOver`.
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER + 1> &result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				callable(
					std::forward<decltype(result[i])>(result[i]),
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER> &result,
			auto &&...others) {
			callable(
				// We believe std::move is necessary here because it makes it into an
				// r-value reference, instead of decaying it into an lvalue, which we
				// don't have an overload for.
				std::move(result),
				std::forward<decltype(others)>(others)...);
		}
		template <
			std::size_t REMAINING_ORDER,
			typename ResultValue,
			std::size_t RESULT_ORDER,
			bool isReducing = (REMAINING_ORDER < RESULT_ORDER),
			typename std::enable_if<isReducing>::type * = nullptr>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, RESULT_ORDER> &result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				Tensor<>::applyOver<REMAINING_ORDER>(
					std::forward<decltype(callable)>(callable),
					std::forward<decltype(result[i])>(result[i]),
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}

		// R-value reference versions of `applyOver`. We cannot have one set of
		// functions call another set, because that degrades the qualifiers on
		// `result`, which need to be exact when passed to `callable`.
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER + 1> &&result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				callable(
					std::forward<decltype(result[i])>(result[i]),
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
		template <std::size_t REMAINING_ORDER, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER> &&result,
			auto &&...others) {
			callable(
				std::forward<decltype(result)>(result),
				std::forward<decltype(others)>(others)...);
		}
		template <
			std::size_t REMAINING_ORDER,
			typename ResultValue,
			std::size_t RESULT_ORDER,
			bool isReducing = (REMAINING_ORDER < RESULT_ORDER),
			typename std::enable_if<isReducing>::type * = nullptr>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, RESULT_ORDER> &&result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				Tensor<>::applyOver<REMAINING_ORDER>(
					std::forward<decltype(callable)>(callable),
					std::forward<decltype(result[i])>(result[i]),
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}

		// Compute the total number of entries in a tensor.
		template <std::size_t TENSOR_ORDER>
		static constexpr inline std::size_t calcSizesProduct(
			std::array<std::size_t, TENSOR_ORDER> const &sizes) {
			std::size_t result{1};
			for (auto const &i : sizes) {
				result *= i;
			}
			return result;
		}
	};

	// Flexible order tensor implementation. Most operations give views of the
	// same underlying data; this is not the case for the binary arithmetic
	// operations.
	template <typename Value, std::size_t ORDER>
	class Tensor {
		// Allow access of any other Tensor for a proper product implementation.
		template <typename, std::size_t>
		friend class Tensor;

		// Allow access from stream operators. It is fine that we only friend one
		// template here, since the template gets expanded into all the versions.
		//
		// Note that the operator is located within the Rain::Math namespace, and
		// this is fine and preferred.
		//
		// Note that we cannot put a default argument for a third argument, simply
		// because this is not a real template definition, but a template
		// declaration.
		template <
			typename TensorValue,
			std::size_t TENSOR_ORDER,
			typename std::enable_if<(TENSOR_ORDER >= 1)>::type *>
		friend std::ostream &operator<<(
			std::ostream &,
			Tensor<TensorValue, TENSOR_ORDER> const &);
		template <typename TensorValue, std::size_t TENSOR_ORDER>
		friend std::istream &operator>>(
			std::istream &,
			Tensor<TensorValue, TENSOR_ORDER> const &);

		private:
		using TypeThis = Tensor<Value, ORDER>;
		using Range = Tensor<>::Range;

		using Error = Tensor<>::Error;
		using Exception = Tensor<>::Exception;

		std::shared_ptr<Value[]> VALUES;
		std::array<Range, ORDER> RANGES;
		// Perceived size based on range.
		std::array<std::size_t, ORDER> SIZES, SIZES_UNDERLYING;
		// Offset caused by previous indexing.
		std::size_t OFFSET;
		// Store a permutation of dimensions for easy transpose & product.
		std::array<std::size_t, ORDER> TRANSPOSE;

		// Only used by the wrapping variant of the scalar constructor.
		Value *P_SCALAR{nullptr};

		static constexpr inline std::array<Range, ORDER> makeRangesDefault(
			std::array<std::size_t, ORDER> const &sizes) {
			std::array<Range, ORDER> ranges;
			for (std::size_t i{0}; i < ORDER; i++) {
				ranges[i] = {0, sizes[i], 1};
			}
			return ranges;
		}
		static constexpr inline std::array<std::size_t, ORDER> calcSizes(
			std::array<Range, ORDER> const &ranges) {
			std::array<std::size_t, ORDER> sizes;
			for (std::size_t i{0}; i < ORDER; i++) {
				sizes[i] = (ranges[i].stop - ranges[i].start + ranges[i].step - 1) /
					ranges[i].step;
			}
			return sizes;
		}
		static constexpr inline std::array<std::size_t, ORDER>
		makeDimPermDefault() {
			std::array<std::size_t, ORDER> dimPerm;
			for (std::size_t i{0}; i < ORDER; i++) {
				dimPerm[i] = i;
			}
			return dimPerm;
		}
		static constexpr inline std::array<std::size_t, ORDER> makeOnesSizes() {
			std::array<std::size_t, ORDER> sizes;
			sizes.fill(1);
			return sizes;
		}

		// DEBUG helpers.
		//
		// Recall that SFINAE requires the expression within `enable_if` to be
		// dependent on a parameter in the current template, which is why we proxy
		// TENSOR_ORDER as the same as ORDER, so that we can use ORDER (through
		// TENSOR_ORDER) in that conditional. Additionally, if the conditional
		// always evaluates true or false, the compiler may refuse to compile.
		template <typename OtherValue>
		inline void debugAssertEqualSizes(
			Tensor<OtherValue, ORDER> const &other) const {
			if (Platform::isDebug()) {
				auto thisSize{this->size()}, otherSize{other.size()};
				for (std::size_t i{0}; i < ORDER; i++) {
					if (thisSize[i] != otherSize[i]) {
						throw Exception(Error::SIZES_MISMATCH);
					}
				}
			}
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		inline void debugAssertSquare() const {
			if (Platform::isDebug()) {
				if (this->SIZES[0] != this->SIZES[1]) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}
		}

		// Additional helpers.

		// Stream out to a std::ostream where each line is padded with additional
		// spaces.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 1)>::type * = nullptr>
		std::ostream &streamOutPadded(std::ostream &stream, std::size_t) const {
			if (this->SIZES[0] == 0) {
				return stream << "[]";
			}
			stream << '[' << std::setw(4) << (*this)[0];
			for (std::size_t i{1}; i < this->SIZES[0]; i++) {
				stream << ' ' << std::setw(4) << (*this)[i];
			}
			return stream << ']';
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER >= 2)>::type * = nullptr>
		std::ostream &streamOutPadded(std::ostream &stream, std::size_t padding)
			const {
			auto size{this->size()};
			if (size[0] == 0) {
				// Attempting to stream a higher order tensor with a 0 dimension will
				// erase traces of the following dimensions.
				return stream << "[]";
			}
			stream << '[';
			(*this)[0].streamOutPadded(stream, padding + 1);
			for (std::size_t i{1}; i < size[0]; i++) {
				stream << "\n" << std::string(padding + 1, ' ');
				(*this)[i].streamOutPadded(stream, padding + 1);
			}
			return stream << ']';
		}

		// Internal constructor for manually specifying underlyings.
		Tensor(
			std::shared_ptr<Value[]> values,
			std::array<Range, ORDER> const &ranges,
			std::array<std::size_t, ORDER> const &sizesUnderlying,
			std::size_t offset,
			std::array<std::size_t, ORDER> const &dimPerm)
				: VALUES{values},
					RANGES{ranges},
					SIZES{TypeThis::calcSizes(ranges)},
					SIZES_UNDERLYING{sizesUnderlying},
					OFFSET{offset},
					TRANSPOSE{dimPerm} {}

		public:
		// Identity, implemented only for matrices.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		static constexpr inline auto identity(std::size_t size) {
			Tensor<Value, 2> result({size, size});
			for (std::size_t i{0}; i < size; i++) {
				result[i][i] = 1;
			}
			return result;
		}

		Tensor(std::array<std::size_t, ORDER> const &sizes, auto &&...values)
				: VALUES{new Value[Tensor<>::calcSizesProduct(sizes)]{
						std::forward<decltype(values)>(values)...}},
					RANGES{TypeThis::makeRangesDefault(sizes)},
					SIZES{sizes},
					SIZES_UNDERLYING{sizes},
					OFFSET{0},
					TRANSPOSE{TypeThis::makeDimPermDefault()} {}
		Tensor(TypeThis const &other)
				: VALUES{other.VALUES},
					RANGES{other.RANGES},
					SIZES{other.SIZES},
					SIZES_UNDERLYING{other.SIZES_UNDERLYING},
					OFFSET{other.OFFSET},
					TRANSPOSE{other.TRANSPOSE} {}

		// The default constructor generates a Tensor with only a single element.
		Tensor() : Tensor(TypeThis::makeOnesSizes()) {}

		// Scalar constructor elides sizes. Allows for free conversion between
		// scalars and Values, alongside the relevant `operator`s.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 0)>::type * = nullptr>
		Tensor(Value const &value) : Tensor({}, value) {}
		// A scalar tensor can also wrap an existing value, but this needs special
		// treatment.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 0)>::type * = nullptr>
		Tensor(Value &value) : Tensor({}) {
			this->P_SCALAR = &value;
		}

		// Copy assignment copies a reference, and not the underlying data.
		//
		// We offer a template but also a specialization to the current class to
		// help compilers recognize the copy assignment operator.
		template <typename OtherValue>
		auto &operator=(Tensor<OtherValue, ORDER> const &other) {
			this->VALUES = other.VALUES;
			this->RANGES = other.RANGES;
			this->SIZES = other.SIZES;
			this->SIZES_UNDERLYING = other.SIZES_UNDERLYING;
			this->OFFSET = other.OFFSET;
			this->TRANSPOSE = other.TRANSPOSE;
			return *this;
		}
		auto &operator=(TypeThis const &other) {
			this->VALUES = other.VALUES;
			this->RANGES = other.RANGES;
			this->SIZES = other.SIZES;
			this->SIZES_UNDERLYING = other.SIZES_UNDERLYING;
			this->OFFSET = other.OFFSET;
			this->TRANSPOSE = other.TRANSPOSE;
			return *this;
		}

		// Equality operator checks sizes and every index.
		template <typename OtherValue>
		bool operator==(Tensor<OtherValue, ORDER> const &other) const {
			auto thisSize{this->size()}, otherSize{other.size()};
			for (std::size_t i{0}; i < ORDER; i++) {
				if (thisSize[i] != otherSize[i]) {
					return false;
				}
			}

			bool isEqual{true};
			Tensor<>::applyOver<0>(
				[&isEqual](Value const &thisValue, OtherValue const &otherValue) {
					isEqual &= thisValue == otherValue;
				},
				*this,
				other);
			return isEqual;
		}

		// A 0-order Tensor converts to/from automatically a scalar. This is
		// necessary for degenerate cases in `product`.
		//
		// Two versions necessary depending on if *this is const or not.
		//
		// It is always constructed without specifying SIZES, and so its underlying
		// size is 1 and must contain a scalar.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 0)>::type * = nullptr>
		inline operator Value const &() const {
			if (this->P_SCALAR != nullptr) {
				return *this->P_SCALAR;
			}
			return this->VALUES[0];
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 0)>::type * = nullptr>
		inline operator Value &() {
			return const_cast<Value &>(this->operator Value const &());
		}

		// Indexing is the main operation of a tensor, and most other operations
		// build on it.
		//
		// Checks in-range iff DEBUG.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 1)>::type * = nullptr>
		inline Value const &operator[](std::size_t idx) const {
			return this->VALUES
				[this->OFFSET + this->RANGES[0].start + this->RANGES[0].step * idx];
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 1)>::type * = nullptr>
		inline Value &operator[](std::size_t idx) {
			return const_cast<Value &>(
				const_cast<TypeThis const *>(this)->operator[](idx));
		}
		// We provide two versions of the higher-order indexing operator, to
		// preserve const-ness.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER > 1)>::type * = nullptr>
		Tensor<Value, ORDER - 1> const operator[](std::size_t idx) const {
			if (Platform::isDebug()) {
				if (idx >= this->SIZES[this->TRANSPOSE[0]]) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}

			// Must take into account transpose.
			std::array<std::size_t, ORDER - 1> newDimPerm;
			for (std::size_t i{1}; i < ORDER; i++) {
				newDimPerm[i - 1] = this->TRANSPOSE[i] +
					(this->TRANSPOSE[i] > this->TRANSPOSE[0] ? -1 : 0);
			}

			std::array<Range, ORDER - 1> ranges;
			std::array<std::size_t, ORDER - 1> sizesUnderlying;
			std::size_t rangeShift{
				this->RANGES[this->TRANSPOSE[0]].start +
				this->RANGES[this->TRANSPOSE[0]].step * idx};
			for (std::size_t i{1}; i < ORDER; i++) {
				sizesUnderlying[newDimPerm[i - 1]] =
					this->SIZES_UNDERLYING[this->TRANSPOSE[i]];
				if (this->TRANSPOSE[i] > this->TRANSPOSE[0]) {
					rangeShift *= this->SIZES_UNDERLYING[this->TRANSPOSE[i]];
					ranges[newDimPerm[i - 1]] = this->RANGES[this->TRANSPOSE[i]];
				} else {
					// `stop` and `start` are calculated from the new `step`.
					ranges[newDimPerm[i - 1]].step =
						this->RANGES[this->TRANSPOSE[i]].step *
						this->SIZES_UNDERLYING[this->TRANSPOSE[0]];
					ranges[newDimPerm[i - 1]].start =
						this->RANGES[this->TRANSPOSE[i]].start *
						this->SIZES_UNDERLYING[this->TRANSPOSE[0]];
					ranges[newDimPerm[i - 1]].stop = ranges[newDimPerm[i - 1]].start +
						this->RANGES[this->TRANSPOSE[i]].step *
							this->SIZES_UNDERLYING[this->TRANSPOSE[0]] *
							this->SIZES[this->TRANSPOSE[i]];
				}
			}
			return {
				this->VALUES,
				ranges,
				sizesUnderlying,
				this->OFFSET + rangeShift,
				newDimPerm};
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER > 1)>::type * = nullptr>
		Tensor<Value, ORDER - 1> operator[](std::size_t idx) {
			return const_cast<Tensor<Value, ORDER - 1> &&>(
				const_cast<TypeThis const *>(this)->operator[](idx));
		}

		// Binary operators. Checks dimension equality iff DEBUG.
		//
		// Binary assignment operators perform operations in-place. Non-assignment
		// versions are constant and will perform allocations of the appropriate
		// size.
		template <typename OtherValue>
		auto operator+(Tensor<OtherValue, ORDER> const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() + std::declval<OtherValue>());
			this->debugAssertEqualSizes(other);
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[](ResultValue &result, Value const &that, OtherValue const &other) {
					result = that + other;
				},
				result,
				*this,
				other);
			return result;
		}
		template <typename OtherValue>
		auto &operator+=(Tensor<OtherValue, ORDER> const &other) {
			Tensor<>::applyOver<0>(
				[](Value &thatValue, OtherValue const &otherValue) {
					thatValue += otherValue;
				},
				*this,
				other);
			return *this;
		}
		template <typename OtherValue>
		auto operator+(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[&other](ResultValue &result, Value const &that) {
					result = that + other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator+=(OtherValue const &other) {
			Tensor<>::applyOver<0>([&other](Value &that) { that += other; }, *this);
			return *this;
		}
		template <typename OtherValue>
		auto operator-(Tensor<OtherValue, ORDER> const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() - std::declval<OtherValue>());
			this->debugAssertEqualSizes(other);
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[](ResultValue &result, Value const &that, OtherValue const &other) {
					result = that - other;
				},
				result,
				*this,
				other);
			return result;
		}
		template <typename OtherValue>
		auto &operator-=(Tensor<OtherValue, ORDER> const &other) {
			Tensor<>::applyOver<0>(
				[](Value &thatValue, OtherValue const &otherValue) {
					thatValue -= otherValue;
				},
				*this,
				other);
			return *this;
		}
		template <typename OtherValue>
		auto operator-(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[&other](ResultValue &result, Value const &that) {
					result = that - other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator-=(OtherValue const &other) {
			Tensor<>::applyOver<0>([&other](Value &that) { that -= other; }, *this);
			return *this;
		}
		// Binary operators *, *= are defined with another Tensor operand iff they
		// are both ORDER 2 and of compatible size. This is a shorthand for
		// `product`.
		template <
			typename OtherValue,
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto operator*(Tensor<OtherValue, ORDER> const &other) const {
			// No need to check sizes here, since `product` will do it.
			return this->product<1>(other, {1}, {0});
		}
		template <
			typename OtherValue,
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto &operator*=(Tensor<OtherValue, ORDER> const &other) {
			// Since allocation will happen anyway, we don't care about doing it
			// in-place.
			return *this = *this * other;
		}
		template <typename OtherValue>
		auto operator*(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[&other](ResultValue &result, Value const &that) {
					result = that * other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator*=(OtherValue const &other) {
			Tensor<>::applyOver<0>([&other](Value &that) { that *= other; }, *this);
			return *this;
		}
		template <typename OtherValue>
		auto operator/(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver<0>(
				[&other](ResultValue &result, Value const &that) {
					result = that / other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator/=(OtherValue const &other) {
			Tensor<>::applyOver<0>([&other](Value &that) { that /= other; }, *this);
			return *this;
		}

		// Memory functions.

		// A tensor is "clean" if its elements are contiguous, in order of
		// dimension (i.e. default start/stop/step/transpose). Usually, transposing
		// or taking a view via slicing will give a "dirty" tensor. However, some
		// views remain clean.
		bool isClean() const {
			std::array<std::size_t, ORDER> transposeSorted{this->TRANSPOSE};
			std::sort(transposeSorted.begin(), transposeSorted.end());
			if (transposeSorted != this->TRANSPOSE) {
				return false;
			}
			for (std::size_t i{0}; i < ORDER; i++) {
				if (
					this->RANGES[i].start != 0 ||
					this->RANGES[i].stop != this->SIZES[i] || this->RANGES[i].step != 1) {
					return false;
				}
			}
			return true;
		}
		auto asClean() const {
			// TODO.
		}
		auto clean() {
			// TODO.
		}
		// Alias for `asClean`.
		auto copy() const {
			// TODO.
		}

		// Property functions.

		// Transposition does not change `SIZES`, and so some functions should use
		// `SIZES`, while others should use `size()`.
		inline std::array<std::size_t, ORDER> size() const {
			std::array<std::size_t, ORDER> sizesTransposed;
			for (std::size_t i{0}; i < ORDER; i++) {
				sizesTransposed[i] = this->SIZES[this->TRANSPOSE[i]];
			}
			return sizesTransposed;
		}
		inline bool isEmpty() const {
			for (auto const &i : this->SIZES) {
				if (i == 0) {
					return true;
				}
			}
			return false;
		}
		// Fills every value with the quality operator.
		void fill(Value const &other) {
			Tensor<>::applyOver<0>([&other](Value &that) { that = other; }, *this);
		}

		// View functions.

		// Slicing returns a view.
		TypeThis asSlice(std::array<Range, ORDER> const &ranges) const {
			std::array<Range, ORDER> mergedRanges;
			for (std::size_t i{0}; i < ORDER; i++) {
				mergedRanges[i].start =
					ranges[i].start == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].start
					: this->RANGES[i].start + this->RANGES[i].step * ranges[i].start;
				mergedRanges[i].stop =
					ranges[i].stop == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].stop
					: this->RANGES[i].start + this->RANGES[i].step * ranges[i].stop;
				mergedRanges[i].step =
					ranges[i].step == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].step
					: this->RANGES[i].step * ranges[i].step;
				// We do not check that start < stop here, since zero-dimensions are
				// technically valid.
			}
			return {
				this->VALUES,
				mergedRanges,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				this->TRANSPOSE};
		}
		TypeThis slice(std::array<Range, ORDER> const &ranges) {
			return *this = this->asSlice(ranges);
		}
		// Must be a valid permutation of [0, ORDER). Checked iff DEBUG.
		TypeThis asTranspose(std::array<std::size_t, ORDER> const &dimPerm) const {
			if (Platform::isDebug()) {
				std::array<std::size_t, ORDER> dimPermSorted{this->TRANSPOSE};
				std::sort(dimPermSorted.begin(), dimPermSorted.end());
				for (std::size_t i{0}; i < ORDER; i++) {
					if (dimPermSorted[i] != i) {
						throw Exception(Error::SIZES_MISMATCH);
					}
				}
			}
			std::array<std::size_t, ORDER> newDimPerm{};
			for (std::size_t i{0}; i < ORDER; i++) {
				newDimPerm[i] = this->TRANSPOSE[dimPerm[i]];
			}
			return {
				this->VALUES,
				this->RANGES,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				newDimPerm};
		}
		TypeThis transpose(std::array<std::size_t, ORDER> const &dimPerm) {
			return *this = this->asTranspose(dimPerm);
		}
		// Reshape preserves the transpose. Use `-1` to attempt to infer the
		// dimension from the total number of entries. Do not use more than one
		// `-1`. Will throw if sizes are incompatible.
		//
		// If Tensor has default start/stop/step and default transpose (i.e. is
		// "clean"), reshape will return a view. Otherwise, reshape
		// returns a copy.
		template <std::size_t NEW_ORDER>
		auto asReshape(std::array<std::size_t, NEW_ORDER> const &sizes) const {
			std::size_t toInfer{NEW_ORDER}, newTotalSize{1},
				totalSize{Tensor<>::calcSizesProduct(this->SIZES)};
			for (std::size_t i{0}; i < NEW_ORDER; i++) {
				if (sizes[i] == -1) {
					if (toInfer != NEW_ORDER) {
						throw Exception(Error::SIZES_MISMATCH);
					}
					toInfer = i;
					continue;
				}
				newTotalSize *= sizes[i];
			}
			if (toInfer != NEW_ORDER) {
				if (totalSize % newTotalSize != 0) {
					throw Exception(Error::SIZES_MISMATCH);
				}
				sizes[toInfer] = totalSize / newTotalSize;
			} else {
				if (totalSize != newTotalSize) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}
			// TODO.
			return Tensor<Value, NEW_ORDER>{
				this->VALUES,
				this->RANGES,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				this->TRANSPOSE};
		}
		template <std::size_t NEW_ORDER>
		auto reshape(std::array<std::size_t, NEW_ORDER> const &sizes) {
			return *this = this->asReshape(sizes);
		}

		// Product functions.

		// Tensor product is defined with a list of pairs of indices to contract.
		//
		// Type of result tensor is the same as the type of Value * OtherValue,
		// without consideration for the contraction + operation.
		//
		// Contraction (+) should be presumed to be commutative. Expansion (*) need
		// not be. Result values are first default-constructed, and should be
		// assumed to be identity w.r.t. contraction.
		//
		// Checks that contraction dimensions are identical iff DEBUG.
		template <
			std::size_t CONTRACT_ORDER,
			template <typename, typename> typename Policy =
				Tensor<>::PlusMultProductPolicy,
			typename OtherValue,
			std::size_t OTHER_ORDER,
			typename ResultValue = decltype(Policy<Value, OtherValue>::product(
				std::declval<Value>(),
				std::declval<OtherValue>())),
			std::size_t RESULT_ORDER = ORDER + OTHER_ORDER - CONTRACT_ORDER * 2>
		auto product(
			Tensor<OtherValue, OTHER_ORDER> const &other,
			std::array<std::size_t, CONTRACT_ORDER> const &thisContractDims,
			std::array<std::size_t, CONTRACT_ORDER> const &otherContractDims) const {
			if (Platform::isDebug()) {
				for (std::size_t i{0}; i < CONTRACT_ORDER; i++) {
					if (
						this->SIZES[this->TRANSPOSE[thisContractDims[i]]] !=
						other.SIZES[other.TRANSPOSE[otherContractDims[i]]]) {
						throw Exception(Error::SIZES_MISMATCH);
					}
				}
			}

			// Transpose all contracted dimensions to the end.
			std::bitset<ORDER> isThisContracted;
			std::bitset<OTHER_ORDER> isOtherContracted;
			for (std::size_t i{0}; i < CONTRACT_ORDER; i++) {
				isThisContracted[thisContractDims[i]] = true;
				isOtherContracted[otherContractDims[i]] = true;
			}
			std::array<std::size_t, ORDER> thisDimPerm;
			std::array<std::size_t, OTHER_ORDER> otherDimPerm;
			for (std::size_t i{0}, j{0}; i < ORDER; i++) {
				if (isThisContracted[i]) {
					continue;
				}
				thisDimPerm[j++] = i;
			}
			for (std::size_t i{0}, j{0}; i < OTHER_ORDER; i++) {
				if (isOtherContracted[i]) {
					continue;
				}
				otherDimPerm[j++] = i;
			}
			for (std::size_t i{CONTRACT_ORDER}; i > 0; i--) {
				thisDimPerm[ORDER - i] = thisContractDims[CONTRACT_ORDER - i];
				otherDimPerm[OTHER_ORDER - i] = otherContractDims[CONTRACT_ORDER - i];
			}
			auto thisTransposed{this->asTranspose(thisDimPerm)};
			auto otherTransposed{other.asTranspose(otherDimPerm)};

			std::array<std::size_t, RESULT_ORDER> resultSize;
			auto thisSize{thisTransposed.size()};
			auto otherSize{otherTransposed.size()};
			for (std::size_t i{0}; i < ORDER - CONTRACT_ORDER; i++) {
				resultSize[i] = thisSize[i];
			}
			for (std::size_t i{0}; i < OTHER_ORDER - CONTRACT_ORDER; i++) {
				resultSize[ORDER - CONTRACT_ORDER + i] = otherSize[i];
			}

			// Iterate over all non-contracted dimensions, and compute contraction
			// (sum) of inner product over all remaining dimensions.
			Tensor<ResultValue, RESULT_ORDER> result(resultSize);
			Tensor<>::applyOver<OTHER_ORDER - CONTRACT_ORDER>(
				[&otherTransposed](
					Tensor<ResultValue, OTHER_ORDER - CONTRACT_ORDER> &&resultOuter,
					Tensor<Value, CONTRACT_ORDER> const &thatInner) {
					Tensor<>::applyOver<0>(
						[&thatInner](
							ResultValue &resultInner,
							Tensor<OtherValue, CONTRACT_ORDER> const &otherInner) {
							// Actually, both `thatInner` and `otherInner` are kept `const`,
							// but we are lazy and don't code the `const` override for
							// `applyOver`.
							resultInner = Policy<Value, OtherValue>::DEFAULT_RESULT;
							Tensor<>::applyOver<0>(
								[&resultInner](
									Value const &thatValue, OtherValue const &otherValue) {
									resultInner = Policy<Value, OtherValue>::sum(
										resultInner,
										Policy<Value, OtherValue>::product(thatValue, otherValue));
								},
								thatInner,
								otherInner);
						},
						resultOuter,
						otherTransposed);
				},
				result,
				thisTransposed);
			return result;
		}
		// Inner and outer product are defined iff ORDER == 1 (vectors).
		template <
			typename OtherValue,
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 1)>::type * = nullptr>
		auto productInner(Tensor<OtherValue, 1> const &other) const {
			return this->product<1>(other, {0}, {0});
		}
		template <
			typename OtherValue,
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 1)>::type * = nullptr>
		auto productOuter(Tensor<OtherValue, 1> const &other) const {
			return this->product<0>(other, {}, {});
		}
		// Square matrices also allow for log power. If not square, `*` will throw
		// iff DEBUG.
		//
		// If the goal is to multiply with a Vector later, it is more efficient (by
		// a constant factor) to manually compute the power product each step along
		// the way. This is because each `1` bit will cause a vector mult as opposed
		// to a matrix mult.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto power(std::size_t exponent) const {
			if (exponent == 0) {
				this->debugAssertSquare();
				return TypeThis::identity(this->SIZES[this->TRANSPOSE[0]]);
			} else if (exponent == 1) {
				return *this;
			}
			auto half{this->power(exponent / 2)};
			if (exponent % 2 == 0) {
				return half * half;
			} else {
				return half * half * *this;
			}
		}
		// Helper function for various recursive algorithms which depend on D&C.
		// Extends the matrix with 1s on the diagonal and 0s everywhere else.
		//
		// May not allocate if this is already a power of 2.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto asNearestPowerOf2() const {
			this->debugAssertSquare();
			auto mSig{Algorithm::mostSignificant1BitIdx(this->SIZES[0])};
			if ((1_zu << mSig) == this->SIZES[0]) {
				return *this;
			}
			auto resultSize{1_zu << (mSig + 1)};
			Tensor<Value, 2> result{{resultSize, resultSize}};
			// Pass `result` second so that we iterate over indices of `*this`.
			Tensor<>::applyOver<0>(
				[](Value &resultValue, Value const &thisValue) {
					resultValue = thisValue;
				},
				result.asSlice({{{1, this->SIZES[0]}, {1, this->SIZES[0]}}}),
				*this);
			return result;
		}
		// Strassen's exists for higher orders, but we do not provide it here.
		// Current Strassen's pads at the beginning, so only use for large matrices
		// where the n^0.19 matters.
		//
		// Strassen's does not work on semi-rings (such as min-plus), and thus does
		// not admit a product policy in this form.
		//
		// Under and including size (1_zu << BASE_SIZE_POWER), Strassen will switch
		// to use standard computation instead.
		template <
			std::size_t BASE_SIZE_POWER = 6,
			typename OtherValue,
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr,
			typename ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>())>
		auto productStrassen(Tensor<OtherValue, 2> const &other) const {
			this->debugAssertSquare();
			other.debugAssertSquare();
			if (Platform::isDebug()) {
				if (this->SIZES[0] != other.SIZES[0]) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}
			if (this->SIZES[0] <= (1_zu << BASE_SIZE_POWER)) {
				return *this * other;
			}
			auto a{this->asNearestPowerOf2()}, b{other.asNearestPowerOf2()};
			auto halfSize{a.SIZES[0] / 2};
			auto a11{a.asSlice({{{0, halfSize}, {0, halfSize}}})},
				a12{a.asSlice({{{0, halfSize}, {halfSize, halfSize * 2}}})},
				a21{a.asSlice({{{halfSize, halfSize * 2}, {0, halfSize}}})},
				a22{a.asSlice({{{halfSize, halfSize * 2}, {halfSize, halfSize * 2}}})},
				b11{b.asSlice({{{0, halfSize}, {0, halfSize}}})},
				b12{b.asSlice({{{0, halfSize}, {halfSize, halfSize * 2}}})},
				b21{b.asSlice({{{halfSize, halfSize * 2}, {0, halfSize}}})},
				b22{b.asSlice({{{halfSize, halfSize * 2}, {halfSize, halfSize * 2}}})};
			std::array<Tensor<ResultValue, 2>, 7> m{
				{{(a11 + a22).productStrassen(b11 + b22)},
				 {(a21 + a22).productStrassen(b11)},
				 {a11.productStrassen(b12 - b22)},
				 {a22.productStrassen(b21 - b11)},
				 {(a11 + a12).productStrassen(b22)},
				 {(a21 - a11).productStrassen(b11 + b12)},
				 {(a12 - a22).productStrassen(b21 + b22)}}};
			Tensor<ResultValue, 2> c{{halfSize * 2, halfSize * 2}};
			Tensor<>::applyOver<0>(
				[](
					ResultValue &c11,
					ResultValue const &m1,
					ResultValue const &m4,
					ResultValue const &m5,
					ResultValue const &m7) { c11 = m1 + m4 - m5 + m7; },
				c.asSlice({{{0, halfSize}, {0, halfSize}}}),
				m[0],
				m[3],
				m[4],
				m[6]);
			Tensor<>::applyOver<0>(
				[](ResultValue &c12, ResultValue const &m3, ResultValue const &m5) {
					c12 = m3 + m5;
				},
				c.asSlice({{{0, halfSize}, {halfSize, halfSize * 2}}}),
				m[2],
				m[4]);
			Tensor<>::applyOver<0>(
				[](ResultValue &c21, ResultValue const &m2, ResultValue const &m4) {
					c21 = m2 + m4;
				},
				c.asSlice({{{halfSize, halfSize * 2}, {0, halfSize}}}),
				m[1],
				m[3]);
			Tensor<>::applyOver<0>(
				[](
					ResultValue &c22,
					ResultValue const &m1,
					ResultValue const &m2,
					ResultValue const &m3,
					ResultValue const &m6) { c22 = m1 - m2 + m3 + m6; },
				c.asSlice({{{halfSize, halfSize * 2}, {halfSize, halfSize * 2}}}),
				m[0],
				m[1],
				m[2],
				m[5]);
			return c.asSlice({{{0, this->SIZES[0]}, {0, this->SIZES[0]}}});
		}

		// Advanced functions.

		// Inverses may exist for higher orders, but we do not provide it here.
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto inverse() const {
			this->debugAssertSquare();

			// TODO: Implement `inverse`.
			return *this;
		}
		template <
			std::size_t TENSOR_ORDER = ORDER,
			typename std::enable_if<(TENSOR_ORDER == 2)>::type * = nullptr>
		auto invert() {
			return *this = this->inverse();
		}
	};

	// Declare the operators here so they can be friended. Streaming a scalar
	// converts automatically to the base type.
	template <
		typename Value,
		std::size_t OTHER_ORDER,
		typename std::enable_if<(OTHER_ORDER >= 1)>::type * = nullptr>
	std::ostream &operator<<(
		std::ostream &stream,
		Rain::Math::Tensor<Value, OTHER_ORDER> const &right) {
		return right.streamOutPadded(stream, 0);
	}
	template <typename Value, std::size_t OTHER_ORDER>
	std::istream &operator>>(
		std::istream &stream,
		Rain::Math::Tensor<Value, OTHER_ORDER> const &) {
		// TODO.
		return stream;
	}
}

#pragma once

#include "../error/exception.hpp"
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
		template <class OtherValue, std::size_t OTHER_ORDER>
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
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		class Range {
			public:
			// `stop`: defaults will be replaced with current `stop` during `slice`.
			// `size`: actual underlying size.
			std::size_t start{std::numeric_limits<std::size_t>::max()},
				stop{std::numeric_limits<std::size_t>::max()},
				step{std::numeric_limits<std::size_t>::max()};
		};

		private:
		// Perform an operation over all indices, or over the first few indices.
		template <std::size_t REMAINING_ORDER = 0, typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, REMAINING_ORDER + 1> result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				callable(
					result[i], std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
		template <
			std::size_t REMAINING_ORDER = 0,
			typename ResultValue,
			std::size_t RESULT_ORDER>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, RESULT_ORDER> result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				applyOver<REMAINING_ORDER>(
					std::forward<decltype(callable)>(callable),
					result[i],
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
	};

	// Flexible order tensor implementation. Most operations give views of the
	// same underlying data; this is not the case for the binary arithmetic
	// operations.
	template <typename Value, std::size_t ORDER>
	class Tensor {
		// Allow access of constructor one dimension down.
		friend Tensor<Value, ORDER + 1>;

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

		static constexpr inline std::size_t calcSizesProduct(
			std::array<std::size_t, ORDER> const &sizes) {
			std::size_t result{1};
			for (auto const &i : sizes) {
				result *= i;
			}
			return result;
		}
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
		Tensor(std::array<std::size_t, ORDER> const &sizes, auto &&...values)
				: VALUES{new Value[TypeThis::calcSizesProduct(sizes)]{
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

		// Checks in-range iff DEBUG.
		template <
			bool isVector = ORDER == 1,
			typename std::enable_if<isVector>::type * = nullptr>
		inline Value &operator[](std::size_t idx) const {
			return this->VALUES
				[this->OFFSET + this->RANGES[0].start + this->RANGES[0].step * idx];
		}
		template <
			bool isNotVector = (ORDER > 1),
			typename std::enable_if<isNotVector>::type * = nullptr>
		Tensor<Value, ORDER - 1> operator[](std::size_t idx) const {
			if (Platform::isDebug()) {
				if (idx >= this->SIZES[this->TRANSPOSE[0]]) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}

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
			Tensor<>::applyOver(
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
			Tensor<>::applyOver(
				[&other](Value &that, OtherValue const &other) { that += other; },
				*this,
				other);
			return *this;
		}
		template <typename OtherValue>
		auto operator+(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver(
				[&other](ResultValue &result, Value const &that) {
					result = that + other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator+=(OtherValue const &other) {
			Tensor<>::applyOver([&other](Value &that) { that += other; }, *this);
			return *this;
		}
		template <typename OtherValue>
		auto operator-(Tensor<OtherValue, ORDER> const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() - std::declval<OtherValue>());
			this->debugAssertEqualSizes(other);
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver(
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
			Tensor<>::applyOver(
				[&other](Value &that, OtherValue const &other) { that -= other; },
				*this,
				other);
			return *this;
		}
		template <typename OtherValue>
		auto operator-(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver(
				[&other](ResultValue &result, Value const &that) {
					result = that - other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator-=(OtherValue const &other) {
			Tensor<>::applyOver([&other](Value &that) { that -= other; }, *this);
			return *this;
		}
		template <typename OtherValue>
		auto operator*(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver(
				[&other](ResultValue &result, Value const &that) {
					result = that * other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator*=(OtherValue const &other) {
			Tensor<>::applyOver([&other](Value &that) { that *= other; }, *this);
			return *this;
		}
		template <typename OtherValue>
		auto operator/(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, ORDER> result(this->size());
			Tensor<>::applyOver(
				[&other](ResultValue &result, Value const &that) {
					result = that / other;
				},
				result,
				*this);
			return result;
		}
		template <typename OtherValue>
		auto &operator/=(OtherValue const &other) {
			Tensor<>::applyOver([&other](Value &that) { that /= other; }, *this);
			return *this;
		}

		// Utility functions.

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
		TypeThis asSlice(std::array<Range, ORDER> &&ranges) const {
			std::array<Range, ORDER> mergedRanges;
			for (std::size_t i{0}; i < ORDER; i++) {
				mergedRanges[i].step =
					ranges[i].step == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].step
					: this->RANGES[i].step * ranges[i].step;
				mergedRanges[i].start =
					ranges[i].start == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].start
					: this->RANGES[i].start + this->RANGES[i].step * ranges[i].start;
				mergedRanges[i].stop =
					ranges[i].stop == std::numeric_limits<std::size_t>::max()
					? this->RANGES[i].stop
					: this->RANGES[i].start + this->RANGES[i].step * ranges[i].stop;
			}
			return {
				this->VALUES,
				mergedRanges,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				this->TRANSPOSE};
		}
		TypeThis slice(std::array<Range, ORDER> &&ranges) {
			return *this = this->asSlice(std::forward<decltype(ranges)>(ranges));
		}
		void fill(Value const &other) {
			Tensor<>::applyOver([&other](Value &that) { that = other; }, *this);
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
			typename OtherValue,
			std::size_t OTHER_ORDER>
		auto product(
			Tensor<OtherValue, OTHER_ORDER> const &other,
			std::array<std::size_t, CONTRACT_ORDER> &&thisContractDims,
			std::array<std::size_t, CONTRACT_ORDER> &&otherContractDims) const {
			using ResultValue =
				decltype(std::declval<Value>() + std::declval<OtherValue>());
			std::size_t const RESULT_ORDER{ORDER + OTHER_ORDER - CONTRACT_ORDER * 2};

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
			std::bitset<ORDER> isThisContracted, isOtherContracted;
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
			auto thisTransposed{this->asTranspose(thisDimPerm)},
				otherTransposed{other.asTranspose(otherDimPerm)};

			std::array<std::size_t, RESULT_ORDER> resultSize;
			auto thisSize{thisTransposed.size()}, otherSize{otherTransposed.size()};
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
					Tensor<ResultValue, OTHER_ORDER - CONTRACT_ORDER> resultOuter,
					Tensor<Value, CONTRACT_ORDER> const &thatInner) {
					Tensor<>::applyOver(
						[&thatInner](
							ResultValue &resultInner,
							Tensor<OtherValue, CONTRACT_ORDER> const &otherInner) {
							// Actually, both `thatInner` and `otherInner` are kept `const`,
							// but we are lazy and don't code the `const` override for
							// `applyOver`.
							Tensor<>::applyOver(
								[&resultInner](
									Value const &thatValue, OtherValue const &otherValue) {
									resultInner += thatValue * otherValue;
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
	};
}

template <typename Value>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Math::Tensor<Value, 1> const &right) {
	using namespace std;
	if (right.isEmpty()) {
		return stream << "[]";
	}
	stream << '[' << setw(4) << right[0];
	for (std::size_t i{1}; i < right.size()[0]; i++) {
		stream << ' ' << setw(4) << right[i];
	}
	return stream << ']';
}

template <
	typename Value,
	std::size_t ORDER,
	bool isNotVector = (ORDER > 1),
	typename std::enable_if<isNotVector>::type * = nullptr>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Math::Tensor<Value, ORDER> const &right) {
	using namespace std;
	if (right.isEmpty()) {
		return stream << "[]";
	}
	stream << '[' << right[0];
	for (std::size_t i{1}; i < right.size()[0]; i++) {
		stream << "\n " << right[i];
	}
	return stream << ']';
}

#pragma once

#include "../error/exception.hpp"
#include "../platform.hpp"

#include <array>
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
		// Perform an operation over all indices.
		template <typename ResultValue>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, 1> result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				callable(
					result[i], std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
		template <typename ResultValue, std::size_t DIM>
		static void applyOver(
			auto &&callable,
			Tensor<ResultValue, DIM> result,
			auto &&...others) {
			for (std::size_t i{0}; i < result.size()[0]; i++) {
				applyOver(
					std::forward<decltype(callable)>(callable),
					result[i],
					std::forward<decltype(others)>(others).operator[](i)...);
			}
		}
	};

	// Tensor operations are generally only checked on dimensionality in DEBUG.
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

			// TODO: This does not handle dimension permutations correctly.
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
				[&other](Value &result, Value const &that, OtherValue const &other) {
					result = that + other;
				},
				*this,
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
			Tensor<>::applyOver(
				[&other](Value &result, Value const &that) { result = that + other; },
				*this,
				*this);
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
				[&other](Value &result, Value const &that, OtherValue const &other) {
					result = that - other;
				},
				*this,
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
			Tensor<>::applyOver(
				[&other](Value &result, Value const &that) { result = that - other; },
				*this,
				*this);
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
			Tensor<>::applyOver(
				[&other](Value &result, Value const &that) { result = that * other; },
				*this,
				*this);
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
			Tensor<>::applyOver(
				[&other](Value &result, Value const &that) { result = that / other; },
				*this,
				*this);
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
		TypeThis asTranspose(std::array<std::size_t, ORDER> &&transpose) const {
			if (Platform::isDebug()) {
				std::array<std::size_t, ORDER> transposeSorted{this->TRANSPOSE};
				std::sort(transposeSorted.begin(), transposeSorted.end());
				for (std::size_t i{0}; i < ORDER; i++) {
					if (transposeSorted[i] != i) {
						throw Exception(Error::SIZES_MISMATCH);
					}
				}
			}
			std::array<std::size_t, ORDER> newDimPerm{};
			for (std::size_t i{0}; i < ORDER; i++) {
				newDimPerm[i] = this->TRANSPOSE[transpose[i]];
			}
			return {
				this->VALUES,
				this->RANGES,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				newDimPerm};
		}
		TypeThis transpose(std::array<std::size_t, ORDER> &&dimPerm) {
			return *this =
							 this->asTranspose(std::forward<decltype(dimPerm)>(dimPerm));
		}
		// Tensor product is defined with a list of pairs of indices to contract.
		template <typename OtherValue, std::size_t OTHER_ORDER>
		void product(
			Tensor<OtherValue, OTHER_ORDER> const &,
			std::array<std::size_t, ORDER> &&) const {
			// Swap all contracted dimensions to the end.

			// Iterate over all non-contracted dimensions, and compute inner product
			// of all remaining dimensions.
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

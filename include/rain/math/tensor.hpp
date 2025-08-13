#pragma once

#include "../error/exception.hpp"

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
		template <class OtherValue, std::size_t OTHER_C_DIM>
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
						return "Tensors are not of compatible size for operation.";
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

	// Tensor operations are generally not bounds-checked.
	template <typename Value, std::size_t C_DIM>
	class Tensor {
		// Allow access of constructor one dimension down.
		friend Tensor<Value, C_DIM + 1>;

		private:
		using TypeThis = Tensor<Value, C_DIM>;
		using Range = Tensor<>::Range;

		using Error = Tensor<>::Error;
		using Exception = Tensor<>::Exception;

		std::shared_ptr<Value[]> const VALUES;
		std::array<Range, C_DIM> const RANGES;
		// Perceived size based on range.
		std::array<std::size_t, C_DIM> const SIZES, SIZES_UNDERLYING;
		// Offset caused by previous indexing.
		std::size_t const OFFSET;
		// Store a permutation of dimensions for easy transpose & product.
		std::array<std::size_t, C_DIM> const DIM_PERM;

		static constexpr inline std::size_t calcSizesProduct(
			std::array<std::size_t, C_DIM> const &sizes) {
			std::size_t result{1};
			for (auto const &i : sizes) {
				result *= i;
			}
			return result;
		}
		static constexpr inline std::array<Range, C_DIM> makeRangesDefault(
			std::array<std::size_t, C_DIM> const &sizes) {
			std::array<Range, C_DIM> ranges;
			for (std::size_t i{0}; i < C_DIM; i++) {
				ranges[i] = {0, sizes[i], 1};
			}
			return ranges;
		}
		static constexpr inline std::array<std::size_t, C_DIM> calcSizes(
			std::array<Range, C_DIM> const &ranges) {
			std::array<std::size_t, C_DIM> sizes;
			for (std::size_t i{0}; i < C_DIM; i++) {
				sizes[i] = (ranges[i].stop - ranges[i].start + ranges[i].step - 1) /
					ranges[i].step;
			}
			return sizes;
		}
		static constexpr inline std::array<std::size_t, C_DIM>
		makeDimPermDefault() {
			std::array<std::size_t, C_DIM> dimPerm;
			for (std::size_t i{0}; i < C_DIM; i++) {
				dimPerm[i] = i;
			}
			return dimPerm;
		}

		Tensor(
			std::shared_ptr<Value[]> values,
			std::array<Range, C_DIM> const &ranges,
			std::array<std::size_t, C_DIM> const &sizesUnderlying,
			std::size_t offset,
			std::array<std::size_t, C_DIM> const &dimPerm)
				: VALUES{values},
					RANGES{ranges},
					SIZES{TypeThis::calcSizes(ranges)},
					SIZES_UNDERLYING{sizesUnderlying},
					OFFSET{offset},
					DIM_PERM{dimPerm} {}

		public:
		Tensor(std::array<std::size_t, C_DIM> const &sizes, auto &&...values)
				: VALUES{new Value[TypeThis::calcSizesProduct(sizes)]{
						std::forward<decltype(values)>(values)...}},
					RANGES{TypeThis::makeRangesDefault(sizes)},
					SIZES{sizes},
					SIZES_UNDERLYING{sizes},
					OFFSET{0},
					DIM_PERM{TypeThis::makeDimPermDefault()} {}
		Tensor(TypeThis const &other)
				: VALUES{other.VALUES},
					RANGES{other.RANGES},
					SIZES{other.SIZES},
					SIZES_UNDERLYING{other.SIZES_UNDERLYING},
					OFFSET{other.OFFSET},
					DIM_PERM{other.DIM_PERM} {}

		// We offer a template but also a specialization to the current class to
		// help compilers recognize the copy assignment operator.
		template <typename OtherValue>
		auto &operator=(Tensor<OtherValue, C_DIM> const &other) {
			Tensor<>::applyOver(
				[](Value &that, OtherValue const &other) { that = other; },
				*this,
				other);
			return *this;
		}
		auto &operator=(TypeThis const &other) {
			Tensor<>::applyOver(
				[](Value &that, Value const &other) { that = other; }, *this, other);
			return *this;
		}

		template <
			bool isVector = C_DIM == 1,
			typename std::enable_if<isVector>::type * = nullptr>
		inline Value &operator[](std::size_t idx) {
			return this->VALUES
				[this->OFFSET + this->RANGES[0].start + this->RANGES[0].step * idx];
		}
		template <
			bool isVector = C_DIM == 1,
			typename std::enable_if<isVector>::type * = nullptr>
		inline Value const &operator[](std::size_t idx) const {
			return const_cast<TypeThis *>(this)->operator[](idx);
		}
		template <
			bool isNotVector = (C_DIM > 1),
			typename std::enable_if<isNotVector>::type * = nullptr>
		Tensor<Value, C_DIM - 1> operator[](std::size_t idx) {
			// TODO: This does not handle dimension permutations correctly.
			std::array<std::size_t, C_DIM - 1> dimPerm;
			for (std::size_t i{1}; i < C_DIM; i++) {
				dimPerm[i - 1] =
					this->DIM_PERM[i] + (this->DIM_PERM[i] > this->DIM_PERM[0] ? -1 : 0);
			}

			std::array<Range, C_DIM - 1> ranges;
			std::array<std::size_t, C_DIM - 1> sizesUnderlying;
			std::size_t rangeShift{
				this->RANGES[this->DIM_PERM[0]].start +
				this->RANGES[this->DIM_PERM[0]].step * idx};
			for (std::size_t i{1}; i < C_DIM; i++) {
				ranges[dimPerm[i - 1]] = this->RANGES[this->DIM_PERM[i]];
				rangeShift *= this->SIZES_UNDERLYING[this->DIM_PERM[i]];
				sizesUnderlying[dimPerm[i - 1]] =
					this->SIZES_UNDERLYING[this->DIM_PERM[i]];
			}
			return {
				this->VALUES,
				ranges,
				sizesUnderlying,
				this->OFFSET + rangeShift,
				dimPerm};
		}
		template <
			bool isNotVector = (C_DIM > 1),
			typename std::enable_if<isNotVector>::type * = nullptr>
		Tensor<Value, C_DIM - 1> const operator[](std::size_t idx) const {
			return const_cast<TypeThis *>(this)->operator[](idx);
		}

		// Binary operators.
		template <typename OtherValue>
		auto operator+(Tensor<OtherValue, C_DIM> const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() + std::declval<OtherValue>());
			Tensor<ResultValue, C_DIM> result(this->SIZES);
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
		auto &operator+=(Tensor<OtherValue, C_DIM> const &other) {
			return *this = *this + other;
		}
		template <typename OtherValue>
		auto operator-(Tensor<OtherValue, C_DIM> const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() - std::declval<OtherValue>());
			Tensor<ResultValue, C_DIM> result(this->SIZES);
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
		auto &operator-=(Tensor<OtherValue, C_DIM> const &other) {
			return *this = *this - other;
		}
		template <typename OtherValue>
		auto operator*(OtherValue const &other) const {
			using ResultValue =
				decltype(std::declval<Value>() * std::declval<OtherValue>());
			Tensor<ResultValue, C_DIM> result(this->SIZES);
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
			return *this = *this * other;
		}

		// Special functions.
		inline std::array<std::size_t, C_DIM> size() const { return this->SIZES; }
		inline bool isEmpty() const {
			bool result{false};
			for (auto const &i : this->size()) {
				result |= i == 0;
			}
			return result;
		}
		TypeThis slice(std::array<Range, C_DIM> &&ranges) {
			std::array<Range, C_DIM> mergedRanges;
			for (std::size_t i{0}; i < C_DIM; i++) {
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
				this->DIM_PERM};
		}
		auto slice(std::array<Range, C_DIM> &&ranges) const {
			return const_cast<TypeThis *>(this)->slice(
				std::forward<decltype(ranges)>(ranges));
		}
		void fill(Value const &other) {
			Tensor<>::applyOver([&other](Value &that) { that = other; }, *this);
		}
		// Must be a valid permutation.
		TypeThis transpose(std::array<std::size_t, C_DIM> &&dimPerm) const {
			return {
				this->VALUES,
				this->RANGES,
				this->SIZES_UNDERLYING,
				this->OFFSET,
				dimPerm};
		}
		// Tensor product is defined with a list of pairs of indices to contract.
		template <typename OtherValue, std::size_t OTHER_C_DIM>
		void product(
			Tensor<OtherValue, OTHER_C_DIM> const &,
			std::array<std::size_t, C_DIM> &&) {
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
	std::size_t C_DIM,
	bool isNotVector = (C_DIM > 1),
	typename std::enable_if<isNotVector>::type * = nullptr>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Math::Tensor<Value, C_DIM> const &right) {
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

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
	};

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

		Tensor(
			std::shared_ptr<Value[]> values,
			std::array<Range, C_DIM> const &ranges,
			std::array<std::size_t, C_DIM> const &sizesUnderlying,
			std::size_t offset)
				: VALUES{values},
					RANGES{ranges},
					SIZES{TypeThis::calcSizes(ranges)},
					SIZES_UNDERLYING{sizesUnderlying},
					OFFSET{offset} {}

		public:
		Tensor(std::array<std::size_t, C_DIM> const &sizes, auto &&...values)
				: VALUES{new Value[TypeThis::calcSizesProduct(sizes)]{
						std::forward<decltype(values)>(values)...}},
					RANGES{TypeThis::makeRangesDefault(sizes)},
					SIZES{sizes},
					SIZES_UNDERLYING{sizes},
					OFFSET{0} {}
		Tensor(std::array<std::size_t, C_DIM> &&sizes, auto &&...values)
				: Tensor(sizes, std::forward<decltype(values)>(values)...) {}

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
			std::array<Range, C_DIM - 1> ranges;
			std::array<std::size_t, C_DIM - 1> sizesUnderlying;
			std::size_t rangeShift{
				this->RANGES[0].start + this->RANGES[0].step * idx};
			for (std::size_t i{1}; i < C_DIM; i++) {
				ranges[i - 1] = this->RANGES[i];
				rangeShift *= this->SIZES_UNDERLYING[i];
				sizesUnderlying[i - 1] = this->SIZES_UNDERLYING[i];
			}
			return {this->VALUES, ranges, sizesUnderlying, this->OFFSET + rangeShift};
		}
		template <
			bool isNotVector = (C_DIM > 1),
			typename std::enable_if<isNotVector>::type * = nullptr>
		Tensor<Value, C_DIM - 1> const operator[](std::size_t idx) const {
			return const_cast<TypeThis *>(this)->operator[](idx);
		}

		// TODO: More operations and validation refactoring.
		void operatorEqualImpl(Tensor<Value, 1> that, Tensor<Value, 1> const &other)
			const {
			for (std::size_t i{0}; i < this->SIZES[C_DIM - 1]; i++) {
				that[i] = other[i];
			}
		}
		template <std::size_t DIM>
		void operatorEqualImpl(
			Tensor<Value, DIM> that,
			Tensor<Value, DIM> const &other) const {
			for (std::size_t i{0}; i < this->SIZES[C_DIM - DIM]; i++) {
				operatorEqualImpl(that[i], other[i]);
			}
		}
		TypeThis &operator=(TypeThis const &other) {
			operatorEqualImpl(*this, other);
			return *this;
		}

		void operatorPlusImpl(
			Tensor<Value, 1> result,
			Tensor<Value, 1> const &that,
			Tensor<Value, 1> const &other) const {
			for (std::size_t i{0}; i < this->SIZES[C_DIM - 1]; i++) {
				result[i] = that[i] + other[i];
			}
		}
		template <std::size_t DIM>
		void operatorPlusImpl(
			Tensor<Value, DIM> result,
			Tensor<Value, DIM> const &that,
			Tensor<Value, DIM> const &other) const {
			for (std::size_t i{0}; i < this->SIZES[C_DIM - DIM]; i++) {
				operatorPlusImpl(result[i], that[i], other[i]);
			}
		}
		TypeThis operator+(TypeThis const &other) const {
			for (std::size_t i{0}; i < C_DIM; i++) {
				if (this->SIZES[i] != other.SIZES[i]) {
					throw Exception(Error::SIZES_MISMATCH);
				}
			}
			TypeThis result(this->SIZES);
			operatorPlusImpl(result, *this, other);
			return result;
		}
		TypeThis &operator+=(TypeThis const &other) {
			return *this = *this + other;
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
			return {this->VALUES, mergedRanges, this->SIZES_UNDERLYING, this->OFFSET};
		}
		auto slice(std::array<Range, C_DIM> &&ranges) const {
			return const_cast<TypeThis *>(this)->slice(
				std::forward<decltype(ranges)>(ranges));
		}
		inline std::array<std::size_t, C_DIM> size() const { return this->SIZES; }
		inline bool isEmpty() const {
			bool result{false};
			for (auto const &i : this->size()) {
				result |= i == 0;
			}
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

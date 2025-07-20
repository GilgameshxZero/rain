#pragma once

#include "point.hpp"

#include <type_traits>
#include <utility>

namespace Rain::Algorithm::Geometry {
	// Represents a close line segment between two points.
	template <typename PrecisionType>
	class LineSegment {
		public:
		Point<PrecisionType> start, end;

		LineSegment() = default;
		LineSegment(
			Point<PrecisionType> const &start,
			Point<PrecisionType> const &end)
				: start{start}, end{end} {}
		LineSegment(
			std::pair<Point<PrecisionType>, Point<PrecisionType>> const &line)
				: start{line.first}, end{line.second} {}

		// Tests whether two non-parallel line segments intersect.
		template <typename OtherPrecisionType>
		inline bool intersects(LineSegment<OtherPrecisionType> const &other) const {
			return (this->end - this->start).crossSign(other.start - this->start) !=
				(this->end - this->start).crossSign(other.end - this->start) &&
				(other.end - other.start).crossSign(this->start - other.start) !=
				(other.end - other.start).crossSign(this->end - other.start);
		}
		template <typename NewPrecisionType>
		inline LineSegment<NewPrecisionType> round() const {
			return {
				this->start.round<NewPrecisionType>(),
				this->end.round<NewPrecisionType>()};
		}
		template <typename NewPrecisionType>
		inline LineSegment<NewPrecisionType> floor() const {
			return {
				this->start.floor<NewPrecisionType>(),
				this->end.floor<NewPrecisionType>()};
		}
		template <typename NewPrecisionType>
		inline LineSegment<NewPrecisionType> ceil() const {
			return {
				this->start.ceil<NewPrecisionType>(),
				this->end.ceil<NewPrecisionType>()};
		}

		inline bool operator==(LineSegment const &other) const {
			return this->start == other.start && this->end == other.end;
		}
		inline bool operator<(LineSegment const &other) const {
			return this->start == other.start ? this->end < other.end
																				: this->start < other.start;
		}
		template <typename OtherPrecisionType>
		inline auto operator+(Point<OtherPrecisionType> const &other) const {
			return LineSegment<decltype(this->start.x + other.x)>{
				this->start + other, this->end + other};
		}
		template <typename ScalarType>
		inline auto operator+(ScalarType const &scalar) const {
			return LineSegment<decltype(this->start.x + scalar)>{
				this->start + scalar, this->end + scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator-(Point<OtherPrecisionType> const &other) const {
			return LineSegment<decltype(this->start.x - other.x)>{
				this->start - other, this->end - other};
		}
		template <typename ScalarType>
		inline auto operator-(ScalarType const &scalar) const {
			return LineSegment<decltype(this->start.x - scalar)>{
				this->start - scalar, this->end - scalar};
		}
		template <typename ScalarType>
		inline auto operator*(ScalarType const &scalar) const {
			return LineSegment<decltype(this->start.x * scalar)>{
				this->start * scalar, this->end * scalar};
		}
		template <typename ScalarType>
		inline auto operator/(ScalarType const &scalar) const {
			return LineSegment<decltype(this->start.x / scalar)>{
				this->start / scalar, this->end / scalar};
		}

		inline operator std::pair<PrecisionType, PrecisionType>() const {
			return {this->start, this->end};
		}

		// Cross-cast integral/non-integral operator.
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			typename std::enable_if<isDifferent>::type * = nullptr>
		explicit inline operator LineSegment<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->start),
				static_cast<OtherPrecisionType>(this->end)};
		}

		// Down-cast is explicit, up-cast is not.
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			bool isSmaller = sizeof(OtherPrecisionType) < sizeof(PrecisionType),
			typename std::enable_if<!isDifferent && isSmaller>::type * = nullptr>
		explicit inline operator LineSegment<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->start),
				static_cast<OtherPrecisionType>(this->end)};
		}
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			bool isSmaller = sizeof(OtherPrecisionType) < sizeof(PrecisionType),
			typename std::enable_if<!isDifferent && !isSmaller>::type * = nullptr>
		inline operator LineSegment<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->start),
				static_cast<OtherPrecisionType>(this->end)};
		}
	};

	using LineSegmentL = LineSegment<long>;
	using LineSegmentLl = LineSegment<long long>;
	using LineSegmentLd = LineSegment<long double>;
}

template <typename PrecisionType>
struct std::hash<Rain::Algorithm::Geometry::LineSegment<PrecisionType>> {
	std::size_t operator()(
		Rain::Algorithm::Geometry::LineSegment<PrecisionType> const &lineSegment)
		const {
		auto hash{std::hash<Rain::Algorithm::Geometry::Point<PrecisionType>>()(
			lineSegment.start)};
		return Rain::Random::combineHash(
			hash,
			std::hash<Rain::Algorithm::Geometry::Point<PrecisionType>>()(
				lineSegment.end));
	}
};

template <typename PrecisionType>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::Geometry::LineSegment<PrecisionType> const &lineSegment) {
	return stream << '(' << lineSegment.start << ", " << lineSegment.end << ')';
}

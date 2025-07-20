#pragma once

#include "../../random.hpp"
#include "../../windows/windows.hpp"
#include "../math.hpp"

#include <type_traits>
#include <utility>

namespace Rain::Algorithm::Geometry {
	template <typename PrecisionType>
	class Point {
		public:
		PrecisionType x, y;

		Point() = default;
		Point(PrecisionType const &x, PrecisionType const &y) : x{x}, y{y} {}
		Point(std::pair<PrecisionType, PrecisionType> const &point)
				: x{point.first}, y{point.second} {}
#ifdef RAIN_PLATFORM_WINDOWS
		template <
			bool isSame = std::is_same<PrecisionType, LONG>::value,
			typename std::enable_if<isSame>::type * = nullptr>
		Point(POINT const &point) : x{point.x}, y{point.y} {}
#endif

		inline long double distanceTo(Point const &other) const {
			return std::sqrtl(
				static_cast<long double>(this->x - other.x) * (this->x - other.x) +
				static_cast<long double>(this->y - other.y) * (this->y - other.y));
		}
		template <typename NewPrecisionType>
		inline Point<NewPrecisionType> round() const {
			return {
				Algorithm::round<NewPrecisionType>(this->x),
				Algorithm::round<NewPrecisionType>(this->y)};
		}
		template <typename NewPrecisionType>
		inline Point<NewPrecisionType> floor() const {
			return {
				Algorithm::floor<NewPrecisionType>(this->x),
				Algorithm::floor<NewPrecisionType>(this->y)};
		}
		template <typename NewPrecisionType>
		inline Point<NewPrecisionType> ceil() const {
			return {
				Algorithm::ceil<NewPrecisionType>(this->x),
				Algorithm::ceil<NewPrecisionType>(this->y)};
		}
		template <typename OtherPrecisionType>
		inline auto cross(Point<OtherPrecisionType> const &other) const {
			return this->x * other.y - this->y * other.x;
		}
		template <typename OtherPrecisionType>
		inline long crossSign(Point<OtherPrecisionType> const &other) const {
			auto cross{this->cross(other)};
			return cross == 0 ? 0 : (cross < 0 ? -1 : 1);
		}

		inline bool operator==(Point const &other) const {
			return this->x == other.x && this->y == other.y;
		}
		inline bool operator<(Point const &other) const {
			return this->x == other.x ? this->y < other.y : this->x < other.x;
		}
		template <typename OtherPrecisionType>
		inline auto operator+(Point<OtherPrecisionType> const &other) const {
			return Point<decltype(this->x + other.x)>{
				this->x + other.x, this->y + other.y};
		}
		template <typename ScalarType>
		inline auto operator+(ScalarType const &scalar) const {
			return Point<decltype(this->x + scalar)>{
				this->x + scalar, this->y + scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator-(Point<OtherPrecisionType> const &other) const {
			return Point<decltype(this->x - other.x)>{
				this->x - other.x, this->y - other.y};
		}
		template <typename ScalarType>
		inline auto operator-(ScalarType const &scalar) const {
			return Point<decltype(this->x - scalar)>{
				this->x - scalar, this->y - scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator*(Point<OtherPrecisionType> const &other) const {
			return Point<decltype(this->x * other.x)>{
				this->x * other.x, this->y * other.y};
		}
		template <typename ScalarType>
		inline auto operator*(ScalarType const &scalar) const {
			return Point<decltype(this->x * scalar)>{
				this->x * scalar, this->y * scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator/(Point<OtherPrecisionType> const &other) const {
			return Point<decltype(this->x / other.x)>{
				this->x / other.x, this->y / other.y};
		}
		template <typename ScalarType>
		inline auto operator/(ScalarType const &scalar) const {
			return Point<decltype(this->x / scalar)>{
				this->x / scalar, this->y / scalar};
		}

		inline operator std::pair<PrecisionType, PrecisionType>() const {
			return {this->x, this->y};
		}
#ifdef RAIN_PLATFORM_WINDOWS
		template <
			bool isSame = std::is_same<PrecisionType, LONG>::value,
			typename std::enable_if<isSame>::type * = nullptr>
		inline operator POINT() const {
			return {this->x, this->y};
		}
#endif

		// Cross-cast integral/non-integral operator.
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			typename std::enable_if<isDifferent>::type * = nullptr>
		explicit inline operator Point<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->x),
				static_cast<OtherPrecisionType>(this->y)};
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
		explicit inline operator Point<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->x),
				static_cast<OtherPrecisionType>(this->y)};
		}
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			bool isSmaller = sizeof(OtherPrecisionType) < sizeof(PrecisionType),
			typename std::enable_if<!isDifferent && !isSmaller>::type * = nullptr>
		inline operator Point<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->x),
				static_cast<OtherPrecisionType>(this->y)};
		}
	};

	using PointL = Point<long>;
	using PointLl = Point<long long>;
	using PointLd = Point<long double>;
}

template <typename PrecisionType>
struct std::hash<Rain::Algorithm::Geometry::Point<PrecisionType>> {
	std::size_t operator()(
		Rain::Algorithm::Geometry::Point<PrecisionType> const &point) const {
		auto hash{Rain::Random::SplitMixHash<PrecisionType>()(point.x)};
		return Rain::Random::combineHash(
			hash, Rain::Random::SplitMixHash<PrecisionType>()(point.y));
	}
};

template <typename PrecisionType>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::Geometry::Point<PrecisionType> const &point) {
	return stream << '(' << point.x << ", " << point.y << ')';
}

#pragma once

#include "../../windows/windows.hpp"
#include "../math.hpp"
#include "point.hpp"

#include <type_traits>
#include <utility>

namespace Rain::Algorithm::Geometry {
	template <typename PrecisionType>
	class Rectangle {
		public:
		PrecisionType left, top, right, bottom;

		Rectangle() = default;
		Rectangle(
			Point<PrecisionType> const &topLeft,
			Point<PrecisionType> const &bottomRight)
				: left{topLeft.x},
					top{topLeft.y},
					right{bottomRight.x},
					bottom{bottomRight.y} {}
		Rectangle(
			PrecisionType const &left,
			PrecisionType const &top,
			PrecisionType const &right,
			PrecisionType const &bottom)
				: left{left}, top{top}, right{right}, bottom{bottom} {}
		Rectangle(
			std::pair<
				std::pair<PrecisionType, PrecisionType>,
				std::pair<PrecisionType, PrecisionType>> const &rectangle)
				: left{rectangle.first.first},
					top{rectangle.first.second},
					right{rectangle.second.first},
					bottom{rectangle.second.second} {}
#ifdef RAIN_PLATFORM_WINDOWS
		template <
			bool isSame = std::is_same<PrecisionType, LONG>::value,
			typename std::enable_if<isSame>::type * = nullptr>
		Rectangle(RECT const &rect)
				: left{rect.left},
					top{rect.top},
					right{rect.right},
					bottom{rect.bottom} {}
#endif

		inline PrecisionType width() const { return this->right - this->left; }
		inline PrecisionType height() const { return this->bottom - this->top; }
		inline Point<PrecisionType> topLeft() const {
			return {this->left, this->top};
		}
		inline Point<PrecisionType> bottomRight() const {
			return {this->right, this->bottom};
		}
		inline PrecisionType area() const { return this->width() * this->height(); }
		inline void include(Point<PrecisionType> const &point) {
			this->left = min(this->left, point.x);
			this->top = min(this->top, point.y);
			this->right = max(this->right, point.x);
			this->bottom = max(this->bottom, point.y);
		}
		inline void expand(PrecisionType const &scalar) {
			this->left -= scalar;
			this->top -= scalar;
			this->right += scalar;
			this->bottom += scalar;
		}
		template <typename NewPrecisionType>
		inline Rectangle<NewPrecisionType> round() const {
			return {
				Algorithm::round<NewPrecisionType>(this->left),
				Algorithm::round<NewPrecisionType>(this->top),
				Algorithm::round<NewPrecisionType>(this->right),
				Algorithm::round<NewPrecisionType>(this->bottom)};
		}
		template <typename NewPrecisionType>
		inline Rectangle<NewPrecisionType> floor() const {
			return {
				Algorithm::floor<NewPrecisionType>(this->left),
				Algorithm::floor<NewPrecisionType>(this->top),
				Algorithm::floor<NewPrecisionType>(this->right),
				Algorithm::floor<NewPrecisionType>(this->bottom)};
		}
		template <typename NewPrecisionType>
		inline Rectangle<NewPrecisionType> ceil() const {
			return {
				Algorithm::ceil<NewPrecisionType>(this->left),
				Algorithm::ceil<NewPrecisionType>(this->top),
				Algorithm::ceil<NewPrecisionType>(this->right),
				Algorithm::ceil<NewPrecisionType>(this->bottom)};
		}
		inline Point<PrecisionType> size() const {
			return {this->width(), this->height()};
		}
		inline Point<PrecisionType> clamp(Point<PrecisionType> const &point) const {
			return {
				std::clamp(point.x, this->left, this->right),
				std::clamp(point.y, this->top, this->bottom)};
		}

		inline bool operator==(Rectangle const &other) const {
			return this->left == other.left && this->top == other.top &&
				this->right == other.right && this->bottom == other.bottom;
		}
		inline bool operator<(Rectangle const &other) const {
			return this->left == other.left
				? (this->top == other.top
						 ? (this->right == other.right ? this->bottom < other.bottom
																					 : this->right < other.right)
						 : this->top < other.top)
				: this->left < other.left;
		}
		template <typename OtherPrecisionType>
		inline auto operator+(Rectangle<OtherPrecisionType> const &other) const {
			return Rectangle<decltype(this->left + other.left)>{
				this->left + other.left,
				this->top + other.top,
				this->right + other.right,
				this->bottom + other.bottom};
		}
		template <typename ScalarType>
		inline auto operator+(ScalarType const &scalar) const {
			return Rectangle<decltype(this->left + scalar)>{
				this->left + scalar,
				this->top + scalar,
				this->right + scalar,
				this->bottom + scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator-(Rectangle<OtherPrecisionType> const &other) const {
			return Rectangle<decltype(this->left + other.left)>{
				this->left - other.left,
				this->top - other.top,
				this->right - other.right,
				this->bottom - other.bottom};
		}
		template <typename ScalarType>
		inline auto operator-(ScalarType const &scalar) const {
			return Rectangle<decltype(this->left - scalar)>{
				this->left - scalar,
				this->top - scalar,
				this->right - scalar,
				this->bottom - scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator*(Rectangle<OtherPrecisionType> const &other) const {
			return Rectangle<decltype(this->left * other.left)>{
				this->left * other.left,
				this->top * other.top,
				this->right * other.right,
				this->bottom * other.bottom};
		}
		template <typename ScalarType>
		inline auto operator*(ScalarType const &scalar) const {
			return Rectangle<decltype(this->left * scalar)>{
				this->left * scalar,
				this->top * scalar,
				this->right * scalar,
				this->bottom * scalar};
		}
		template <typename OtherPrecisionType>
		inline auto operator/(Rectangle<OtherPrecisionType> const &other) const {
			return Rectangle<decltype(this->left / other.left)>{
				this->left / other.left,
				this->top / other.top,
				this->right / other.right,
				this->bottom / other.bottom};
		}
		template <typename ScalarType>
		inline auto operator/(ScalarType const &scalar) const {
			return Rectangle<decltype(this->left / scalar)>{
				this->left / scalar,
				this->top / scalar,
				this->right / scalar,
				this->bottom / scalar};
		}

		inline operator std::pair<
			std::pair<PrecisionType, PrecisionType>,
			std::pair<PrecisionType, PrecisionType>>() const {
			return {{this->left, this->top}, {this->right, this->bottom}};
		}
#ifdef RAIN_PLATFORM_WINDOWS
		template <
			bool isSame = std::is_same<PrecisionType, LONG>::value,
			typename std::enable_if<isSame>::type * = nullptr>
		inline operator RECT() const {
			return {this->left, this->top, this->right, this->bottom};
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
		explicit inline operator Rectangle<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->left),
				static_cast<OtherPrecisionType>(this->top),
				static_cast<OtherPrecisionType>(this->right),
				static_cast<OtherPrecisionType>(this->bottom)};
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
		explicit inline operator Rectangle<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->left),
				static_cast<OtherPrecisionType>(this->top),
				static_cast<OtherPrecisionType>(this->right),
				static_cast<OtherPrecisionType>(this->bottom)};
		}
		template <
			typename OtherPrecisionType,
			bool isCurrentIntegral = std::is_integral<PrecisionType>::value,
			bool isOtherIntegral = std::is_integral<PrecisionType>::value,
			bool isDifferent = (isCurrentIntegral && !isOtherIntegral) ||
				(!isCurrentIntegral && isOtherIntegral),
			bool isSmaller = sizeof(OtherPrecisionType) < sizeof(PrecisionType),
			typename std::enable_if<!isDifferent && !isSmaller>::type * = nullptr>
		inline operator Rectangle<OtherPrecisionType>() const {
			return {
				static_cast<OtherPrecisionType>(this->left),
				static_cast<OtherPrecisionType>(this->top),
				static_cast<OtherPrecisionType>(this->right),
				static_cast<OtherPrecisionType>(this->bottom)};
		}
	};

	using RectangleS = Rectangle<short>;
	using RectangleL = Rectangle<long>;
	using RectangleLl = Rectangle<long long>;
	using RectangleLd = Rectangle<long double>;
}

template <typename PrecisionType>
struct std::hash<Rain::Algorithm::Geometry::Rectangle<PrecisionType>> {
	std::size_t operator()(
		Rain::Algorithm::Geometry::Rectangle<PrecisionType> const &rectangle)
		const {
		auto hash{Rain::Random::SplitMixHash<PrecisionType>()(rectangle.left)};
		Rain::Random::combineHash(
			hash, Rain::Random::SplitMixHash<PrecisionType>()(rectangle.top));
		Rain::Random::combineHash(
			hash, Rain::Random::SplitMixHash<PrecisionType>()(rectangle.right));
		return Rain::Random::combineHash(
			hash, Rain::Random::SplitMixHash<PrecisionType>()(rectangle.bottom));
	}
};

template <typename PrecisionType>
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Algorithm::Geometry::Rectangle<PrecisionType> const &rectangle) {
	return stream << "(" << rectangle.topLeft() << ", " << rectangle.bottomRight()
								<< ')';
}

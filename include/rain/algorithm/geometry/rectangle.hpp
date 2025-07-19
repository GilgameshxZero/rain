#pragma once

#include "../../windows/windows.hpp"
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

	using RectangleI = Rectangle<int>;
	using RectangleL = Rectangle<long>;
	using RectangleS = Rectangle<std::size_t>;
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

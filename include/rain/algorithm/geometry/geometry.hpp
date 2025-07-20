#pragma once

#include "point.hpp"

namespace Rain::Algorithm::Geometry {
	template <typename PrecisionType>
	inline long sign(PrecisionType const &a) {
		return a == 0 ? 0 : (a < 0 ? -1 : 1);
	}

	// Cross product of vectors (b - a) and (c - a) (in that order).
	template <typename PrecisionType>
	inline long double crossProduct(
		Point<PrecisionType> const &a,
		Point<PrecisionType> const &b,
		Point<PrecisionType> const &c) {
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	template <typename PrecisionType>
	inline bool isSegmentsIntersecting(
		Point<PrecisionType> const &a,
		Point<PrecisionType> const &b,
		Point<PrecisionType> const &c,
		Point<PrecisionType> const &d) {
		return sign(crossProduct(a, b, c)) != sign(crossProduct(a, b, d)) &&
			sign(crossProduct(c, d, a)) != sign(crossProduct(c, d, b));
	}
}

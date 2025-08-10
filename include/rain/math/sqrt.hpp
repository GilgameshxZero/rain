#pragma once

namespace Rain::Math {
	// `constexpr` sqrt by <https://stackoverflow.com/a/27709195>.
	template <typename T>
	constexpr T sqrtImpl(T x, T lo, T hi) {
		if (lo == hi) {
			return lo;
		}

		const T mid = (lo + hi + 1) / 2;
		if (x / mid < mid) {
			return sqrtImpl<T>(x, lo, mid - 1);
		} else {
			return sqrtImpl(x, mid, hi);
		}
	}
	template <typename T>
	constexpr T sqrt(T x) {
		return sqrtImpl<T>(x, 0, x / 2 + 1);
	}
}

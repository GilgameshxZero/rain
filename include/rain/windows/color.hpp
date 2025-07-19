#pragma once

#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	class Color {
		public:
		uint8_t const r, g, b, a;

		Color() = default;
		Color(COLORREF const &color)
				: r{GetRValue(color)},
					g{GetGValue(color)},
					b{GetBValue(color)},
					a{255} {}
		Color(uint8_t r, uint8_t g, uint8_t b) : r{r}, g{g}, b{b}, a{255} {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
				: r{r}, g{g}, b{b}, a{a} {}
		Color(int i)
				: r{static_cast<uint8_t>(i >> 16)},
					g{static_cast<uint8_t>(i >> 8)},
					b{static_cast<uint8_t>(i)},
					a{static_cast<uint8_t>(i >> 24)} {}

		operator COLORREF() const { return RGB(this->r, this->g, this->b); }
	};
}

#endif

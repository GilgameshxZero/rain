#pragma once

#include "../algorithm/geometry.hpp"
#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	class Bitmap {
		private:
		HBITMAP const hBitmap;

		public:
		Bitmap(HDC hDc, int cx, int cy)
				: hBitmap{validateSystemCall(CreateCompatibleBitmap(hDc, cx, cy))} {}
		~Bitmap() { DeleteObject(this->hBitmap); }
		Bitmap(Bitmap const &other) = delete;
		Bitmap &operator=(Bitmap const &other) = delete;
		Bitmap(Bitmap &&other) = delete;
		Bitmap &operator=(Bitmap &&other) = delete;

		inline operator HBITMAP() const { return this->hBitmap; }

		inline Algorithm::Geometry::PointL getDimension() const {
			SIZE size;
			validateSystemCall(GetBitmapDimensionEx(*this, &size));
			return {size.cx, size.cy};
		}
	};
}

#endif

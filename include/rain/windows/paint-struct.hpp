#pragma once

#include "../algorithm/geometry.hpp"
#include "device-context.hpp"
#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	// Manages BeginPaint and EndPaint in a WM_PAINT handler.
	class PaintStruct {
		private:
		HWND hWnd;
		PAINTSTRUCT ps;

		public:
		PaintStruct(HWND hWnd) : hWnd{hWnd} {
			validateSystemCall(BeginPaint(hWnd, &this->ps));
		}
		~PaintStruct() { EndPaint(this->hWnd, &this->ps); }
		PaintStruct(PaintStruct const &) = delete;
		PaintStruct &operator=(PaintStruct const &) = delete;
		PaintStruct(PaintStruct &&) = delete;
		PaintStruct &operator=(PaintStruct &&) = delete;

		inline operator PAINTSTRUCT() const { return this->ps; }

		DeviceContext hDc() const { return {this->ps.hdc}; }
		Algorithm::Geometry::RectangleL rcPaint() const { return {this->ps.rcPaint}; }
	};
}

#endif

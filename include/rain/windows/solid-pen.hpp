#pragma once

#include "color.hpp"
#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	class SolidPen {
		private:
		HPEN const hPen;

		public:
		SolidPen(int width, Color color)
				: hPen{validateSystemCall(CreatePen(PS_SOLID, width, color))} {}
		~SolidPen() { DeleteObject(this->hPen); }
		SolidPen(SolidPen const &other) = delete;
		SolidPen &operator=(SolidPen const &other) = delete;
		SolidPen(SolidPen &&other) = delete;
		SolidPen &operator=(SolidPen &&other) = delete;

		inline operator HPEN() const { return this->hPen; }
	};
}

#endif

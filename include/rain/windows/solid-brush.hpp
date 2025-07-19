#pragma once

#include "color.hpp"
#include "exception.hpp"
#include "windows.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows {
	class SolidBrush {
		private:
		HBRUSH const hBrush;

		public:
		SolidBrush(Color color)
				: hBrush{validateSystemCall(CreateSolidBrush(color))} {}
		~SolidBrush() { DeleteObject(this->hBrush); }
		SolidBrush(SolidBrush const &other) = delete;
		SolidBrush &operator=(SolidBrush const &other) = delete;
		SolidBrush(SolidBrush &&other) = delete;
		SolidBrush &operator=(SolidBrush &&other) = delete;

		inline operator HBRUSH() const { return this->hBrush; }
	};
}

#endif

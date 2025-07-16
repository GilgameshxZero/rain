// Include Gdiplus while linking the correct libraries and resolving min/max.
#pragma once

#include "../../platform.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

#pragma comment(lib, "Gdiplus.lib")

#include "../../algorithm/algorithm.hpp"
#include "../windows.hpp"

// min/max are defined within Gdiplus as well, and this avoids double definition
// errors.
namespace Gdiplus {
	using std::max;
	using std::min;
}

// Objidl.h and Windows.h must be included before Gdiplus.h.
#include <Objidl.h>

#include <Gdiplus.h>

// Calls GdiplusStartup on construct, GdiplusShutdown on destruct.
namespace Rain::Windows::Gdiplus {
	class AutoGdiplusManager {
		private:
		class _ {
			public:
			::Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;

			_() { GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, 0); };
			~_() { ::Gdiplus::GdiplusShutdown(this->gdiplusToken); }
		};

		// Constructed during global initialization (before main), destructed after
		// main.
		static inline _ invoker;

		public:
		static ULONG_PTR &gdiplusToken() {
			return AutoGdiplusManager::invoker.gdiplusToken;
		}
	};
}

// Custom hashes and equality.
template <>
struct std::hash<Gdiplus::Point> {
	std::size_t operator()(Gdiplus::Point const &point) const {
		return std::hash<int>()(point.X) ^ (std::hash<int>()(point.Y) << 1);
	}
};

template <>
struct std::equal_to<Gdiplus::Point> {
	bool operator()(Gdiplus::Point const &left, Gdiplus::Point const &right)
		const {
		return left.X == right.X && left.Y == right.Y;
	}
};

#endif

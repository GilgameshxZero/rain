#pragma once

#include "./platform.hpp"
#include "./windows.hpp"

#ifdef RAIN_WINDOWS
#pragma comment(lib, "Gdiplus.lib")
#include <algorithm>

namespace Gdiplus {
	using std::max;
	using std::min;
}

// Must be in this order.
#include <Objidl.h>

#include <Gdiplus.h>
#endif

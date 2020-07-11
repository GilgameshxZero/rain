// Include Gdiplus while linking the correct libraries and resolving min/max.
#pragma once

#include "platform.hpp"
#include "windows.hpp"

#ifdef RAIN_WINDOWS
#pragma comment(lib, "Gdiplus.lib")

// Bypasses some errors with min and max being defined twice.
#include <algorithm>
namespace Gdiplus {
	using std::max;
	using std::min;
}

// Must be in this order.
#include <Objidl.h>

#include <Gdiplus.h>
#endif

// Include Gdiplus while linking the correct libraries and resolving min/max.
#pragma once

#include "platform.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

#pragma comment(lib, "Gdiplus.lib")

#include "algorithm/algorithm.hpp"
#include "windows/windows.hpp"

// min/max are defined within Gdiplus as well, and this avoids double definition
// errors.
namespace Gdiplus {
	using std::max;
	using std::min;
}

// Objidl.h and Windows.h must be included before Gdiplus.h.
#include <Objidl.h>

#include <Gdiplus.h>

#endif

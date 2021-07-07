/*
Include Gdiplus while linking the correct libraries and resolving min/max.
*/

#pragma once

#include "platform.hpp"
#include "windows.hpp"

// Inform the linker we expect this library.
#ifdef RAIN_WINDOWS
#pragma comment(lib, "Gdiplus.lib")

#ifndef NOMINMAX
#define NOMINMAX
#endif

/*
min/max are defined within Gdiplus as well, and this avoids double definition errors.
*/
#include "algorithm.hpp"

namespace Gdiplus {
	using std::max;
	using std::min;
}

/*
These includes must be in this order.
TODO: Why?
*/
#include <Objidl.h>

#include <Gdiplus.h>

#endif

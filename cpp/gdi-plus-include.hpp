/*
rain-aeternum

Gdiplus programs can be finnicky with includes and defines. Use this as the
common include file.
*/

#pragma once

#pragma comment(lib, "Gdiplus.lib")

#include "windows-lam-include.hpp"

#include <algorithm>

namespace Gdiplus {
	using std::max;
	using std::min;
}

// Must come before Gdiplus.h. Empty line prevents clang from modifying order.
#include <Objidl.h>

#include <Gdiplus.h>

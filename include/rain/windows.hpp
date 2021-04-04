// Include Windows.h with WIN32_LEAN_AND_MEAN for WSA2 library includes later
// on.

#pragma once

#include "platform.hpp"

// Excludes default Winsock implementations.
#ifdef RAIN_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#endif

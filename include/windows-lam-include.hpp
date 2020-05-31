/*
Include this instead of Windows.h, to prevent problems when using WSA2
libraries.
*/

#pragma once

#include "platform.hpp"

#ifdef RAIN_WINDOWS

// Need defines before include.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#endif

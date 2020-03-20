/*
Include this instead of Windows.h, to prevent problems when using WSA2
libraries.
*/

#pragma once

#include "utility-platform.hpp"

#ifdef RAIN_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#endif

// Includes Windows.h headers in a way that it is impossible to interfere with
// Windows-related APIs used elsewhere in Rain. Including this on a non-Windows
// platform does nothing.
#pragma once

#include "platform.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

// Rain utilizes Unicode in system calls, UTF-8 internally.
#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

// Prevents Windows.h from automatically include Winsock 1.1; Rain uses
// Winsock 2.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// The minimum version of Windows that Rain can run on is Windows 7.
#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#endif

// Prevents double declarations of min/max functions.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#endif

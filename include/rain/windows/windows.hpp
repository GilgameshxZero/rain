// Includes Windows.h headers in a way that it is impossible to interfere with
// Windows-related APIs used elsewhere in Rain. Including this on a non-Windows
// platform does nothing.
#pragma once

#include "../platform.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

// Rain utilizes ANSI Windows functions internally.
#ifdef UNICODE
#undef UNICODE
#endif

// Prevents Windows.h from automatically include Winsock 1.1; Rain uses
// Winsock 2.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Prevents double declarations of min/max functions.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")

#include <Windows.h>
#include <shellapi.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif

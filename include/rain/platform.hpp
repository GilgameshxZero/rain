/*
Utilities for cross-platform development.
*/

#pragma once

#include <string>

// Standardize preprocessors.
#ifdef __CYGWIN__
#define RAIN_CYGWIN
#endif

#if defined(_WIN32) || defined(_WIN64)
#define RAIN_WINDOWS
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
#define RAIN_UNIX
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define RAIN_MAC
#endif

#if defined(__linux__) || defined(linux) || defined(__linux)
#define RAIN_LINUX
#endif

#ifdef __FreeBSD__
#define RAIN_FREEBSD
#endif

#ifdef __ANDROID__
#define RAIN_ANDROID
#endif

namespace Rain {
	bool isCygwin();
	bool isWindows();
	bool isUnix();
	bool isMac();
	bool isLinux();
	bool isFreeBSD();
	bool isAndroid();

	std::string getPlatformString();
}

// Utilities for platform-detecting, and declarations for low-level platform
// specific functions.
#pragma once

// Platform preprocessors.
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

// Include some GNU extensions.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdlib>
#include <cstring>

namespace Rain::Platform {
	inline const char *getPlatformString() {
#ifdef RAIN_CYGWIN
		return "Cygwin";
#elif defined(RAIN_WINDOWS)
		return "Windows";
#elif defined(RAIN_UNIX)
		return "Unix";
#elif defined(RAIN_MAC)
		return "Mac";
#elif defined(RAIN_LINUX)
		return "Linux";
#elif defined(RAIN_FREEBSD)
		return "FreeBSD";
#elif defined(RAIN_ANDROID)
		return "Android";
#endif
		return "";
	}
}

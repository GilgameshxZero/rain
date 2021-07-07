/*
Utilities for platform-detecting, and declarations for low-level platform
specific functions.
*/

#pragma once

// Platform preprocessors.
#ifdef __CYGWIN__
#define RAIN_CYGWIN
#endif

#if defined(_WIN32) || defined(_WIN64)
#define RAIN_WINDOWS
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define RAIN_MACOS
#endif

#if defined(__linux__) || defined(linux) || defined(__linux)
#define RAIN_LINUX
#endif

namespace Rain::Platform {
	inline char const *getPlatformCStr() noexcept {
#ifdef RAIN_CYGWIN
		return "Cygwin";
#elif defined(RAIN_WINDOWS)
		return "Windows";
#elif defined(RAIN_MACOS)
		return "MacOS";
#elif defined(RAIN_LINUX)
		return "Linux";
#endif
		return "Other";
	}
}

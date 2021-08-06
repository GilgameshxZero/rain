// Utilities for platform and build detection.
//
// Rain supports distinguishing between 3 major platforms: Windows, MacOS, and
// Linux-based.
#pragma once

#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#define RAIN_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define RAIN_PLATFORM_MACOS
#endif

#if defined(__linux__) || defined(linux) || defined(__linux)
#define RAIN_PLATFORM_LINUX
#endif

#if !defined(RAIN_PLATFORM_WINDOWS) && !defined(RAIN_PLATFORM_MACOS) && \
	!defined(RAIN_PLATFORM_LINUX)
#define RAIN_PLATFORM_OTHER
#endif

#ifdef NDEBUG
#define RAIN_DEBUG_NDEBUG
#endif

namespace Rain::Platform {
	enum class Platform { NONE = 0, WINDOWS, MACOS, LINUX };

	// Get the Platform enum Rain is running on.
	inline Platform getPlatform() noexcept {
#ifdef RAIN_PLATFORM_WINDOWS
		return Platform::WINDOWS;
#elif defined(RAIN_PLATFORM_MACOS)
		return Platform::MACOS;
#elif defined(RAIN_PLATFORM_LINUX)
		return Platform::LINUX;
#else
		return Platform::NONE;
#endif
	}

	// Returns whether the code was built in debug mode.
	inline bool isDebug() noexcept {
#ifdef RAIN_DEBUG_NDEBUG
		return false;
#else
		return true;
#endif
	}
}

// Ease-of-stream for Rain::Platform::Platform.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Platform::Platform platform) {
	switch (platform) {
		case Rain::Platform::Platform::WINDOWS:
			stream << "Windows";
			break;
		case Rain::Platform::Platform::MACOS:
			stream << "MacOS";
			break;
		case Rain::Platform::Platform::LINUX:
			stream << "Linux";
			break;
		default:
			stream << "None";
			break;
	}

	return stream;
}

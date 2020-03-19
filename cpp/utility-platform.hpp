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
	bool isCygwin() {
#ifdef RAIN_CYGWIN
		return true;
#endif
		return false;
	}

	bool isWindows() {
#ifdef RAIN_WINDOWS
		return true;
#endif
		return false;
	}

	bool isUnix() {
#ifdef RAIN_UNIX
		return true;
#endif
		return false;
	}

	bool isMac() {
#ifdef RAIN_MAC
		return true;
#endif
		return false;
	}

	bool isLinux() {
#ifdef RAIN_LINUX
		return true;
#endif
		return false;
	}

	bool isFreeBSD() {
#ifdef RAIN_FREEBSD
		return true;
#endif
		return false;
	}

	bool isAndroid() {
#ifdef RAIN_ANDROID
		return true;
#endif
		return false;
	}

	std::string getPlatformString() {
#if defined(RAIN_CYGWIN)
		return "Cygwin";
#elif defined(RAIN_WINDOWS)
		return "Windows";
#elif defined(RAIN_UNIX)
		return "Unix";
#elif defined(RAIN_MAC)
		return "Mac OS";
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

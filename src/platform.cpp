#include "rain/platform.hpp"

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

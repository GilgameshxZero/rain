/*
Utilities for debugging.
*/

#pragma once

#ifdef NDEBUG
#define RAIN_NDEBUG
#endif

namespace Rain::Debug {
	/*
	Returns whether the code was built in debug mode.
	*/
	inline bool isDebug() noexcept {
#ifdef RAIN_NDEBUG
		return false;
#else
		return true;
#endif
	}
}

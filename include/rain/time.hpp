/*
Compatibility layer for differences between platforms.
*/

#pragma once

#include "platform.hpp"

#include <ctime>
#include <thread>

namespace Rain {
#ifdef RAIN_WINDOWS
	inline tm *localtime_r(time_t const *const _clock, tm *const _result) {
		localtime_s(_result, _clock);
		return _result;
	}
#endif
}

namespace Rain::Time {
	// Shorthand for sleeping the current thread.
	inline void sleepMs(std::size_t ms) noexcept {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}

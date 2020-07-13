// Compatibility layer for differences between platforms.
#pragma once

#include "platform.hpp"

#include <ctime>
#include <thread>

namespace Rain {
#ifdef RAIN_WINDOWS
	inline struct tm *localtime_r(time_t *_clock, struct tm *_result) {
		localtime_s(_result, _clock);
		return _result;
	}
#endif
}

namespace Rain::Time {
	inline void sleep(std::size_t ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}

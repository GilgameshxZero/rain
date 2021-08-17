// Normalizes platform differences in time functions and provides general
// utilities for time manipulation.
#pragma once

#include "../platform.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

namespace Rain::Time {
	// Transform time to printable string.
	//
	// Provided by Rain on Windows, available on POSIX.
	inline tm *localtime_r(time_t const *_clock, tm *_result) {
#ifdef RAIN_PLATFORM_WINDOWS
		localtime_s(_result, _clock);
		return _result;
#else
		return ::localtime_r(_clock, _result);
#endif
	}
}

// All durations are outputted as ms.
template <typename TickRep, typename TickPeriod>
inline std::ostream &operator<<(
	std::ostream &stream,
	std::chrono::duration<TickRep, TickPeriod> duration) {
	return stream << std::chrono::duration_cast<
										 std::chrono::duration<long double, std::milli>>(duration)
										 .count()
								<< "ms";
}

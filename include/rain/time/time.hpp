// Normalizes platform differences in time functions and provides general
// utilities for time manipulation.
#pragma once

#include "../platform.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

namespace Rain::Time {
#ifdef RAIN_PLATFORM_WINDOWS
	// Transform time to printable string.
	//
	// Provided by Rain on Windows, available on POSIX.
	inline tm *localtime_r(time_t const *const _clock, tm *const _result) {
		localtime_s(_result, _clock);
		return _result;
	}
#endif
}

// Ease-of-stream for common durations.
inline std::ostream &operator<<(
	std::ostream &stream,
	std::chrono::nanoseconds const &ns) {
	return stream << ns.count() << "ns";
}
inline std::ostream &operator<<(
	std::ostream &stream,
	std::chrono::milliseconds const &ms) {
	return stream << ms.count() << "ms";
}
inline std::ostream &operator<<(std::ostream &stream, std::chrono::seconds const &s) {
	return stream << s.count() << "s";
}

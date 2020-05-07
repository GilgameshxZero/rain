/*
Utility functions related to time.
*/

#pragma once

#include "utility-platform.hpp"

#include <time.h>
#include <mutex>
#include <string>
#include <thread>

namespace Rain {
#ifdef RAIN_WINDOWS
	struct tm *localtime_r(time_t *_clock, struct tm *_result);
#endif

	// Convenience functions for getting time in an std::string.
	std::string getTime(time_t rawTime, std::string format = "%F %T%z");
	std::string getTime(std::string format = "%F %T%z");

	void sleep(int ms);
}

#pragma once

#include <ctime>
#include <thread>

namespace Rain::Time {
	inline void sleep(size_t ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}

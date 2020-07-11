#pragma once

#include "../platform/.hpp"

#include <ctime>

namespace Rain {
	// Compatibility.
#ifdef RAIN_WINDOWS
	inline struct tm *localtime_r(time_t *_clock, struct tm *_result) {
		localtime_s(_result, _clock);
		return _result;
	}
#endif
}

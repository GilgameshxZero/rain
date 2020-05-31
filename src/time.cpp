#include "time.hpp"

namespace Rain {
#ifdef RAIN_WINDOWS
	struct tm *localtime_r(time_t *_clock, struct tm *_result) {
		localtime_s(_result, _clock);
		return _result;
	}
#endif

	std::string getTime(time_t rawTime, std::string format) {
		struct tm timeInfo;
		char buffer[128];
		localtime_r(&rawTime, &timeInfo);
		strftime(buffer, sizeof(buffer), format.c_str(), &timeInfo);
		return buffer;
	}
	std::string getTime(std::string format) {
		time_t rawTime;
		time(&rawTime);
		return getTime(rawTime, format);
	}

	void sleep(int ms) {
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}
}

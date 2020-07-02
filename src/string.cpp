#include "rain/string.hpp"

namespace Rain {
	errno_t strcpy_s(char *dest, rsize_t destsz, const char *src) {
#ifdef RAIN_WINDOWS
		return ::strcpy_s(dest, destsz, src);
#else
		strncpy(dest, src, destsz)[destsz - 1] = '\0';
		return 0;
#endif
	}
}
#pragma once

#include "rain/platform.hpp"

#include <cstring>
#include <string>

namespace Rain {
	typedef size_t rsize_t;
	typedef int errno_t;

	errno_t strcpy_s(char *dest, rsize_t destsz, const char *src);
}
// Normalizes platform differences in type availability.
#pragma once

#include <cstdlib>

namespace Rain {
	// errno_t is non-standard; however we define it here to maximize
	// compatibility with overloading library functions which return this.
	typedef int errno_t;
}

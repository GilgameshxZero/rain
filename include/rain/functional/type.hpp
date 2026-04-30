// Normalizes platform differences in type availability.
#pragma once

#include <cstdint>

namespace Rain {
	// errno_t is non-standard; however we define it here to
	// maximize compatibility with overloading library
	// functions which return this.
	using errno_t = int;
}

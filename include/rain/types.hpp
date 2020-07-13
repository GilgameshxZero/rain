#pragma once

#include <cuchar>

namespace Rain {
	typedef int errno_t;

	// Literal for size_t types.
	// Does not work with Windows platforms.
	constexpr std::size_t operator"" _SZ(unsigned long long x) { return x; }
}

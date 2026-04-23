// Utilities for assert during runtime & testing.
#pragma once

#include "../platform.hpp"
#include "../string/stringify.hpp"

#include <cstdlib>
#include <iostream>
#include <source_location>

// Retrieves caller site for debugging.
//
// Superceded by `std::source_location::current()` in C++20.
#define RAIN_ERROR_LOCATION __FILE__ ":" STRINGIFY(__LINE__)

namespace Rain::Error {
	// `std::abort()` if false, otherwise no-op.
	inline void releaseAssert(
		bool condition,
		std::source_location location =
			std::source_location::current()) {
		if (!condition) {
			std::cerr << "Assert failed: " << location.file_name()
								<< ':' << location.line() << " in "
								<< location.function_name() << '.'
								<< std::endl;
			std::abort();
		}
	}

	// Assert is a no-op if NDEBUG is defined.
	//
	// Must name differently from `assert` macro defined in
	// <cassert>.
	inline void debugAssert(bool condition) {
		if (Rain::Platform::isDebug()) {
			releaseAssert(condition);
		}
	}
}

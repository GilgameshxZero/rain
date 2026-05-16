// Normalizes platform differences in type availability, and
// defines common types to all of Rain.
#pragma once

#include <cstdint>

namespace Rain {
	// errno_t is non-standard; however we define it here to
	// maximize compatibility with overloading library
	// functions which return this.
	using errno_t = int;

	// Standard types whose template parameters are types,
	// under the "everything-as-a-type" paradigm of
	// `trait.hpp`.
	//
	// TODO.
}

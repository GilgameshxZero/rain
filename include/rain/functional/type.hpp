// Normalizes platform differences in type availability.
#pragma once

#include "trait.hpp"

#include <cstdint>
#include <type_traits>

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

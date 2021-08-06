// Includes <algorithm> and prevents min/max ambiguity.
#pragma once

// Skip over definitions of global min/max from other sources.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>

// Override global min/max with std::min/std::max to resolve any compilation
// ambiguity.
using std::min;
using std::max;

/*
Replaces the <algorithm> include in STL to resolve any issues with double-defining min/max.
*/

#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>

using std::min;
using std::max;

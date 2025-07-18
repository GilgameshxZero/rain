// Includes all rain libraries.
#pragma once

// Versioning.
#define RAIN_VERSION_MAJOR 7
#define RAIN_VERSION_MINOR 2
#define RAIN_VERSION_REVISION 34

// Disable secure warnings; the caller should be aware of out-of-bounds errors
// when calling any not-bound-checked function.
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "rain/build.hpp"

#include "rain/algorithm.hpp"
#include "rain/error.hpp"
#include "rain/filesystem.hpp"
#include "rain/literal.hpp"
#include "rain/log.hpp"
#include "rain/multithreading.hpp"
#include "rain/networking.hpp"
#include "rain/platform.hpp"
#include "rain/string.hpp"
#include "rain/time.hpp"
#include "rain/type.hpp"
#include "rain/windows.hpp"

/*
This include should include all current and legacy Rain library headers.

If possible, Rain functions should be provided in two versions: for pointer
parameters, and for non-pointer parameters. Sometimes, copy constructors are
slow, so using pointer versions of functions will be faster.

To use the libraries, all files in the Rain library directory must be added to
the project (externally is fine). To include this file, either some project
settings must be changed or use a relative/absolute include path. Suggestion:
place Rain library directory (e.g. RainAeternum/RainLibrary3) under solution
directory.

All functions are thread-safe unless otherwise mentioned.
*/

#pragma once

#define RAIN_VERSION_MAJOR 3
#define RAIN_VERSION_MINOR 0
#define RAIN_VERSION_REVISION 3

#include "rain-build.hpp"

// These includes must be in this order.
#include "gdi-plus-include.hpp"
#include "network-wsa-include.hpp"
#include "windows-lam-include.hpp"

#include "rain-window.hpp"

#include "algorithm-libraries.hpp"
#include "condition-variable.hpp"
#include "configuration.hpp"
#include "gdi-plus-libraries.hpp"
#include "network-libraries.hpp"
#include "timer.hpp"
#include "utility-libraries.hpp"

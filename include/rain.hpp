// Includes all rain libraries.
#pragma once

// This is necessary for struct addrinfo.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Versioning.
#define RAIN_VERSION_MAJOR 6
#define RAIN_VERSION_MINOR 0
#define RAIN_VERSION_REVISION 13

#include "rain/build.hpp"

#include "rain/algorithm/kmp.hpp"
#include "rain/networking/custom-server.hpp"
#include "rain/networking/native-socket.hpp"
#include "rain/networking/node-service-host.hpp"
#include "rain/networking/server.hpp"
#include "rain/networking/socket.hpp"
#include "rain/string/command-line-parser.hpp"
#include "rain/string/waterfall-parser.hpp"

#include "rain/gdiplus.hpp"
#include "rain/memmem.hpp"
#include "rain/platform.hpp"
#include "rain/string.hpp"
#include "rain/thread-pool.hpp"
#include "rain/time.hpp"
#include "rain/types.hpp"
#include "rain/windows.hpp"

#include <iostream>

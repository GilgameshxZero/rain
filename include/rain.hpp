// Includes all rain libraries.
#pragma once

// This is necessary for struct addrinfo.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Versioning.
#define RAIN_VERSION_MAJOR 6
#define RAIN_VERSION_MINOR 1
#define RAIN_VERSION_REVISION 1

#include "rain/build.hpp"

#include "rain/algorithm/kmp.hpp"
#include "rain/algorithm/lru.hpp"
#include "rain/filesystem.hpp"
#include "rain/gdiplus.hpp"
#include "rain/memmem.hpp"
#include "rain/networking/custom-server.hpp"
#include "rain/networking/http/body.hpp"
#include "rain/networking/http/client.hpp"
#include "rain/networking/http/header.hpp"
#include "rain/networking/http/payload-request-response.hpp"
#include "rain/networking/http/server.hpp"
#include "rain/networking/http/socket.hpp"
#include "rain/networking/native-socket.hpp"
#include "rain/networking/node-service-host.hpp"
#include "rain/networking/server.hpp"
#include "rain/networking/smtp/client.hpp"
#include "rain/networking/smtp/payload-request-response.hpp"
#include "rain/networking/smtp/server.hpp"
#include "rain/networking/smtp/socket.hpp"
#include "rain/networking/socket.hpp"
#include "rain/platform.hpp"
#include "rain/string.hpp"
#include "rain/string/command-line-parser.hpp"
#include "rain/string/waterfall-parser.hpp"
#include "rain/thread-pool.hpp"
#include "rain/time.hpp"
#include "rain/types.hpp"
#include "rain/windows.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

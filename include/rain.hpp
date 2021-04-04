// Includes all rain libraries.
#pragma once

// This is necessary for struct addrinfo.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// Versioning.
#define RAIN_VERSION_MAJOR 6
#define RAIN_VERSION_MINOR 5
#define RAIN_VERSION_REVISION 0

#include "rain/build.hpp"

#include "rain/algorithm.hpp"
#include "rain/algorithm/kmp.hpp"
#include "rain/algorithm/lru.hpp"
#include "rain/error-exception.hpp"
#include "rain/filesystem.hpp"
#include "rain/gdiplus.hpp"
#include "rain/memmem.hpp"
#include "rain/networking/client.hpp"
#include "rain/networking/host.hpp"
#include "rain/networking/http/body.hpp"
#include "rain/networking/http/client.hpp"
#include "rain/networking/http/header.hpp"
#include "rain/networking/http/payload.hpp"
#include "rain/networking/http/request.hpp"
#include "rain/networking/http/response.hpp"
#include "rain/networking/http/server.hpp"
#include "rain/networking/http/slave.hpp"
#include "rain/networking/http/socket.hpp"
#include "rain/networking/native-socket.hpp"
#include "rain/networking/request-response/client.hpp"
#include "rain/networking/request-response/request.hpp"
#include "rain/networking/request-response/response.hpp"
#include "rain/networking/request-response/server.hpp"
#include "rain/networking/request-response/slave.hpp"
#include "rain/networking/request-response/socket.hpp"
#include "rain/networking/server.hpp"
#include "rain/networking/slave.hpp"
#include "rain/networking/smtp/client.hpp"
#include "rain/networking/smtp/payload.hpp"
#include "rain/networking/smtp/request.hpp"
#include "rain/networking/smtp/response.hpp"
#include "rain/networking/smtp/server.hpp"
#include "rain/networking/smtp/slave.hpp"
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

// Provides platform-independent NativeSocket and NATIVE_SOCKET_INVALID.
#pragma once

#include "../platform.hpp"

#ifdef RAIN_WINDOWS
#include "../windows.hpp"

#ifndef _WIN32_WINNT
// Windows XP.
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#pragma comment(lib, "Ws2_32.lib")
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
// Assume that any non-Windows platform uses POSIX-style sockets.
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Rain::Networking {
#ifdef RAIN_WINDOWS
	typedef SOCKET NativeSocket;
	static NativeSocket const NATIVE_SOCKET_INVALID = INVALID_SOCKET;
	static int const NATIVE_SOCKET_ERROR = SOCKET_ERROR;
#else
	typedef int NativeSocket;
	static NativeSocket const NATIVE_SOCKET_INVALID = -1;
	static int const NATIVE_SOCKET_ERROR = -1;
#endif
}

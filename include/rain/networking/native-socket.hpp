// Includes all relevant headers to implement the NativeSocket type, which
// abstracts away the platform differences between naive socket handles.
#pragma once

#include "../platform.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

// Links ws2_32.dll which includes implementation for Winsock 2. Not necessarily
// x86 as the name suggests.
#pragma comment(lib, "Ws2_32.lib")

#include "../windows.hpp"

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else

// Non-Windows networking is supported via POSIX-style sockets.
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

namespace Rain::Networking {
	// Abstraction of platform socket. Error/invalid differences between
	// platforms.
#ifdef RAIN_PLATFORM_WINDOWS
	typedef SOCKET NativeSocket;
	static NativeSocket const NATIVE_SOCKET_INVALID = INVALID_SOCKET;
	static int const NATIVE_SOCKET_ERROR = SOCKET_ERROR;
#else
	typedef int NativeSocket;
	static NativeSocket const NATIVE_SOCKET_INVALID = -1;
	static int const NATIVE_SOCKET_ERROR = -1;
#endif
}

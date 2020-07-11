#pragma once

#include "../platform/.hpp"
#include "../string/.hpp"

#ifdef RAIN_WINDOWS
#include "../windows/.hpp"

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
	// Internal platform-specific implementation.
#ifdef RAIN_WINDOWS
	typedef SOCKET NativeSocket;
	typedef int AddressLength;

	static const SOCKET INVALID_NATIVE_SOCKET = SOCKET_ERROR;
#else
	typedef int NativeSocket;
	typedef socklen_t AddressLength;

	static const int INVALID_NATIVE_SOCKET = -1;
#endif

	class Host {
		public:
		const char *node, *service;

		Host(const char *node, const char *service)
				: node(node), service(service) {}
		Host(const char *node = NULL, size_t service = 80)
				: Host(node, String::tToStr(service).c_str()) {}
	};
}

#pragma once

#include "rain/platform.hpp"
#include "rain/string.hpp"
#include "rain/windows.hpp"

#ifdef RAIN_WINDOWS
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

namespace Rain {
	class Socket {
		public:
		Socket();
		~Socket();

#ifdef RAIN_WINDOWS
		static bool wsaInitialized;
		static WSADATA wsaData;

		static int wsaCleanup();
#endif

#ifdef RAIN_WINDOWS
		SOCKET socket;
#else
		int socket;
#endif

#ifdef RAIN_WINDOWS
		SOCKET create(int = AF_INET, int = SOCK_STREAM, int = IPPROTO_TCP);
#else
		int create(int = AF_INET, int = SOCK_STREAM, int = IPPROTO_TCP);
#endif
		int connect(const char *, const char *);
		int bind();
		int listen();
		int accept();
		int send(const void *, size_t, int = 0);
		int send(const char *, int = 0);
		int recv(void *, size_t, int = 0);
		int close();

		private:
	};
}

/*
A platform-agnostic socket implementation.
*/

#pragma once

#include "./platform.hpp"
#include "./string.hpp"
#include "./windows.hpp"

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
		// Internal platform-specific implementation.
#ifdef RAIN_WINDOWS
		typedef SOCKET NativeSocket;
		typedef int AddressLength;
#else
		typedef int NativeSocket;
		typedef socklen_t AddressLength;
#endif
		NativeSocket socket;

		// Stored from constructor.
		int family, type, protocol;

		Socket(NativeSocket socket = 0,
			int family = AF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP)
				: socket(socket), family(family), type(type), protocol(protocol) {
#ifdef RAIN_WINDOWS
			Socket::wsaStartup();
#endif
			if (this->socket == 0) {
				this->socket = ::socket(family, type, protocol);
			}
		}
		~Socket() {}

		// Stuff related to WSA in Windows.
#ifdef RAIN_WINDOWS
		inline static bool wsaInitialized = false;
		inline static WSADATA wsaData;

		static int wsaStartup() {
			if (!Socket::wsaInitialized) {
				Socket::wsaInitialized = true;
				return WSAStartup(MAKEWORD(2, 2), &Socket::wsaData);
			}
			return -1;
		}
		static int wsaCleanup() { return WSACleanup(); }
#endif

		// Shorthand for calling wsaCleanup in Windows and doing nothing otherwise.
		static int cleanup() {
#ifdef RAIN_WINDOWS
			return wsaCleanup();
#else
			return 0;
#endif
		}

		// Platform-agnostic socket operations adapted to the Socket interface.
		int connect(const char *node, const char *service) {
			struct addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = this->family;
			hints.ai_socktype = this->type;
			hints.ai_protocol = this->protocol;
			int status = getaddrinfo(node, service, &hints, &result);
			if (status != 0) {
				return status;
			}

			curAddr = result;
			while (curAddr != NULL) {
				status = ::connect(this->socket,
					curAddr->ai_addr,
					static_cast<int>(curAddr->ai_addrlen));
				if (status == 0) {
					return 0;
				}
				curAddr = curAddr->ai_next;
			}
			return status;
		}
		int bind(const char *service) {
			struct addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = this->family;
			hints.ai_socktype = this->type;
			hints.ai_protocol = this->protocol;
			hints.ai_flags = AI_PASSIVE;
			int status = getaddrinfo(NULL, service, &hints, &result);
			if (status != 0) {
				return status;
			}

			curAddr = result;
			while (curAddr != NULL) {
				status = ::bind(this->socket,
					curAddr->ai_addr,
					static_cast<int>(curAddr->ai_addrlen));
				if (status == 0) {
					return 0;
				}
				curAddr = curAddr->ai_next;
			}
			return status;
		}
		int listen(int backlog = 1024) { return ::listen(this->socket, backlog); }

		Socket accept(struct sockaddr *addr = NULL, AddressLength *addrLen = NULL) {
			return Socket(::accept(this->socket, addr, addrLen),
				this->family,
				this->type,
				this->protocol);
		}

		int send(const void *msg, size_t len, int flags = 0) {
			return ::send(this->socket,
#ifdef RAIN_WINDOWS
				reinterpret_cast<const char *>(msg),
				static_cast<int>(len),
#else
				msg,
				len,
#endif
				flags);
		}
		int send(const char *msg, int flags = 0) {
			return this->send(
				reinterpret_cast<const void *>(msg), strlen(msg), flags);
		}
		int recv(void *buf, size_t len, int flags = 0) {
			return ::recv(this->socket,
#ifdef RAIN_WINDOWS
				reinterpret_cast<char *>(buf),
				static_cast<int>(len),
#else
				buf,
				len,
#endif
				flags);
		}
		int close() {
			int status;
#ifdef RAIN_WINDOWS
			status = shutdown(this->socket, SD_BOTH);
			if (status == 0) {
				status = closesocket(this->socket);
			}
#else
			status = shutdown(this->socket, SHUT_RDWR);
			if (status == 0) {
				status = ::close(this->socket);
			}
#endif
			return status;
		}
	};
}

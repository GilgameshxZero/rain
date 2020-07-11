/*
A platform-agnostic socket implementation.
*/

#pragma once

#include "../platform/.hpp"
#include "./networking.hpp"

#include <functional>

namespace Rain::Networking {
	class Socket {
		public:
		NativeSocket socket;
		int family, type, protocol;

		// Sockets are created in an invalid state.
		Socket(bool create = false,
			NativeSocket socket = INVALID_NATIVE_SOCKET,
			int family = AF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP)
				: socket(socket), family(family), type(type), protocol(protocol) {
			Socket::startup();

			if (create) {
				this->create();
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
			return 0;
		}
		static int wsaCleanup() { return WSACleanup(); }
#endif

		// Shorthand for calling wsaCleanup in Windows and doing nothing otherwise.
		static int startup() {
#ifdef RAIN_WINDOWS
			return Socket::wsaStartup();
#else
			return 0;
#endif
		}
		static int cleanup() {
#ifdef RAIN_WINDOWS
			return Socket::wsaCleanup();
#else
			return 0;
#endif
		}

		NativeSocket create() {
			return this->socket = ::socket(this->family, this->type, this->protocol);
		}

		// Platform-agnostic socket operations adapted to the Socket interface.
		int connect(Host host) {
			return this->connectOrBind(
				host.node, host.service, [](struct addrinfo *) {}, ::connect);
		}

		int bind(Host host) {
			return this->connectOrBind(
				host.node,
				host.service,
				[](struct addrinfo *hints) { hints->ai_flags = AI_PASSIVE; },
				::bind);
		}

		int listen(int backlog = 1024) { return ::listen(this->socket, backlog); }

		NativeSocket accept(struct sockaddr *addr = NULL,
			AddressLength *addrLen = NULL) {
			return ::accept(this->socket, addr, addrLen);
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
		int send(const std::string *s, int flags = 0) {
			return this->send(s->c_str(), flags);
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

		private:
		// Shared code for bind and connect.
		int connectOrBind(const char *node,
			const char *service,
			std::function<void(struct addrinfo *)> setHints,
			std::function<int(NativeSocket, sockaddr *, int)> action) {
			struct addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = this->family;
			hints.ai_socktype = this->type;
			hints.ai_protocol = this->protocol;
			setHints(&hints);
			int status = getaddrinfo(node, service, &hints, &result);
			if (status != 0) {
				return status;
			}

			curAddr = result;
			while (curAddr != NULL) {
				status = action(this->socket,
					curAddr->ai_addr,
					static_cast<int>(curAddr->ai_addrlen));
				if (status == 0) {
					return 0;
				}
				curAddr = curAddr->ai_next;
			}
			return status;
		}
	};
}

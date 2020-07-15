// A platform-agnostic OOP socket implementation.

#pragma once

#include "../platform.hpp"
#include "native-socket.hpp"
#include "node-service-host.hpp"

#include <functional>

#include <iostream>

namespace Rain::Networking {
	class Socket {
		public:
		NativeSocket socket;
		int family, type, protocol;

		// Sockets are created in an invalid state.
		Socket(bool create = false,
			NativeSocket socket = NATIVE_SOCKET_INVALID,
			int family = AF_INET,
			int type = SOCK_STREAM,
			int protocol = IPPROTO_TCP)
				: socket(socket), family(family), type(type), protocol(protocol) {
			Socket::startup();

			if (create) {
				this->create();
			}
		}

		// Stuff related to WSA in Windows.
#ifdef RAIN_WINDOWS
		inline static bool wsaInitialized = false;
		inline static WSADATA wsaData;
#endif

		// Shorthand for calling wsaCleanup in Windows and doing nothing otherwise.
		static int startup() {
#ifdef RAIN_WINDOWS
			if (!Socket::wsaInitialized) {
				Socket::wsaInitialized = true;
				return WSAStartup(MAKEWORD(2, 2), &Socket::wsaData);
			}
#endif
			return 0;
		}
		static int cleanup() {
#ifdef RAIN_WINDOWS
			return WSACleanup();
#else
			return 0;
#endif
		}

		NativeSocket create() {
			return this->socket = ::socket(this->family, this->type, this->protocol);
		}

		// Platform-agnostic socket operations adapted to the Socket interface.
		int connect(const Host &host) const noexcept {
			return this->connectOrBind(
				host, [](struct addrinfo *) {}, ::connect);
		}

		int bind(const Host &host) const noexcept {
			return this->connectOrBind(
				host,
				[](struct addrinfo *hints) { hints->ai_flags = AI_PASSIVE; },
				::bind);
		}

		int listen(int backlog = 1024) const noexcept {
			return ::listen(this->socket, backlog);
		}

		NativeSocket accept(struct sockaddr *addr = NULL,
			socklen_t *addrLen = NULL) const noexcept {
			return ::accept(this->socket, addr, addrLen);
		}

		int send(const void *msg, std::size_t len, int flags = MSG_NOSIGNAL) const
			noexcept {
			int status = ::send(this->socket,
#ifdef RAIN_WINDOWS
				reinterpret_cast<const char *>(msg),
				static_cast<int>(len),
#else
				msg,
				len,
#endif
				flags);
			std::cout << status << std::endl;
			return status;
		}
		int send(const char *msg, int flags = MSG_NOSIGNAL) const noexcept {
			return this->send(
				reinterpret_cast<const void *>(msg), strlen(msg), flags);
		}
		int send(const std::string &s, int flags = MSG_NOSIGNAL) const noexcept {
			return this->send(s.c_str(), flags);
		}

		int recv(void *buf, std::size_t len, int flags = 0) const noexcept {
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

		// Shutdown should be called on any connected sockets (not listening
		// sockets). Returns nonzero on error.
		int shutdown() const noexcept {
#ifdef RAIN_WINDOWS
			static const int how = SD_BOTH;
#else
			static const int how = SHUT_RDWR;
#endif
			return ::shutdown(this->socket, how);
		}

		// Tries to shutdown and then closes. Returns nonzero on error. If shutdown
		// fails, will just try to close.
		int close() const noexcept {
			this->shutdown();
#ifdef RAIN_WINDOWS
			return closesocket(this->socket);
#else
			return ::close(this->socket);
#endif
		}

		private:
		// Shared code for bind and connect.
		int connectOrBind(const Host &host,
			std::function<void(struct addrinfo *)> setHints,
			std::function<int(NativeSocket, sockaddr *, int)> action) const noexcept {
			struct addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = this->family;
			hints.ai_socktype = this->type;
			hints.ai_protocol = this->protocol;
			setHints(&hints);
			int status = getaddrinfo(
				host.node.getCStr(), host.service.getCStr(), &hints, &result);
			if (status != 0) {
				return status;
			}

			// Try all the addresses we found.
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

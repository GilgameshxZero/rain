#include "rain/socket.hpp"

namespace Rain {
#ifdef RAIN_WINDOWS
	bool Socket::wsaInitialized;
	WSADATA Socket::wsaData;
#endif

	Socket::Socket() : socket(0) {
#ifdef RAIN_WINDOWS
		if (!Socket::wsaInitialized) {
			Socket::wsaInitialized = true;
			WSAStartup(MAKEWORD(2, 2), &Socket::wsaData);
		}
#endif
	}
	Socket::~Socket() {}

#ifdef RAIN_WINDOWS
	int Socket::wsaCleanup() { return WSACleanup(); }
#endif

#ifdef RAIN_WINDOWS
	SOCKET Socket::create(int family, int type, int protocol) {
#else
	int Socket::create(int family, int type, int protocol) {
#endif
		return this->socket = ::socket(family, type, protocol);
	}

	int Socket::connect(const char *node, const char *service) {
		struct addrinfo hints, *result, *curAddr;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (0 != getaddrinfo(node, service, &hints, &result)) {
			return 1;
		}

		curAddr = result;
		while (curAddr != NULL) {
			if (0 ==
				::connect(this->socket,
					curAddr->ai_addr,
					static_cast<int>(curAddr->ai_addrlen))) {
				return 0;
			}
			curAddr = curAddr->ai_next;
		}
		return 2;
	}
	int Socket::send(const void *msg, size_t len, int flags) {
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
	int Socket::send(const char *msg, int flags) {
		return this->send(reinterpret_cast<const void *>(msg), strlen(msg), flags);
	}
	int Socket::recv(void *buf, size_t len, int flags) {
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
	int Socket::close() { return 0; }
}

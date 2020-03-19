/*
Standard
*/

/*
Basic functions for connecting sockets and sending over them.

Higher-level, optional functions are in NetworkUtility.
*/

#pragma once

#include "network-wsa-include.hpp"
#include "rain-window.hpp"

#include <string>
#include <unordered_map>

namespace Rain {
	// shortcut function for starting WSA2.2
	int initWinsock22();

	// target returned needs to be freed
	// if no error, returns 0
	int getTargetAddr(struct addrinfo **target,
			std::string host,
			std::string port,
			int family = AF_UNSPEC,
			int sockType = SOCK_STREAM,
			int type = IPPROTO_TCP);

	// same thing as getTargetAddr, but with different default arguments, to be
	// used for servers trying to listen on a port return 0 if no error
	int getServerAddr(struct addrinfo **server,
			std::string port,
			int family = AF_INET,
			int sockType = SOCK_STREAM,
			int type = IPPROTO_TCP,
			int flags = AI_PASSIVE);

	// if no error, returns 0
	int createSocket(SOCKET &newSocket,
			int family = AF_UNSPEC,
			int sockType = SOCK_STREAM,
			int type = IPPROTO_TCP);

	// run through all the target possibilities and try to connect to any of them
	// if no error, returns 0
	int connectTarget(SOCKET &cSocket, struct addrinfo **target);

	// binds a socket to a local address
	// returns 0 if no error
	int bindListenSocket(struct addrinfo **addr, SOCKET &lSocket);

	// returns a new socket for a connected client
	// blocks until a client connects, or until blocking functions are called off
	SOCKET acceptClientSocket(SOCKET &lSocket);

	// frees address gotten by getTargetAddr or getServerAddr
	void freeAddrInfo(struct addrinfo **addr);

	// if no error, returns 0
	int shutdownSocketSend(SOCKET &cSocket);

	// send raw message over a socket
	// returns 0 if no error
	int sendRawMessage(SOCKET &sock, const char *cstrtext, int len);
	int sendRawMessage(SOCKET &sock, std::string strText);
	int sendRawMessage(SOCKET &sock, std::string *strText);
}

namespace Rain {
	int initWinsock22() {
		WSADATA wsaData;
		return WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	int getTargetAddr(struct addrinfo **target,
			std::string host,
			std::string port,
			int family,
			int sockType,
			int type) {
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = sockType;
		hints.ai_protocol = type;

		return getaddrinfo(host.c_str(), port.c_str(), &hints, target);
	}
	int getServerAddr(struct addrinfo **server,
			std::string port,
			int family,
			int sockType,
			int type,
			int flags) {
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = sockType;
		hints.ai_protocol = type;
		hints.ai_flags = flags;

		return getaddrinfo(NULL, port.c_str(), &hints, server);
	}
	int createSocket(SOCKET &newSocket, int family, int sockType, int type) {
		newSocket = socket(family, sockType, type);
		if (newSocket == INVALID_SOCKET)
			return WSAGetLastError();
		return 0;
	}
	int connectTarget(SOCKET &cSocket, struct addrinfo **target) {
		struct addrinfo *curaddr;
		int ret;

		curaddr = (*target);
		while (true) {
			ret = connect(cSocket, curaddr->ai_addr, (int)curaddr->ai_addrlen);
			if (ret == SOCKET_ERROR)
				ret = WSAGetLastError();
			else
				break;

			if (curaddr->ai_next == NULL)
				break;
			curaddr = curaddr->ai_next;
		}

		return ret;
	}
	int bindListenSocket(struct addrinfo **addr, SOCKET &lSocket) {
		if (bind(lSocket, (*addr)->ai_addr, (int)(*addr)->ai_addrlen))
			return -1;
		if (listen(lSocket, SOMAXCONN))
			return -1;
		return 0;
	}
	SOCKET acceptClientSocket(SOCKET &lSocket) {
		return accept(lSocket, NULL, NULL);
	}
	void freeAddrInfo(struct addrinfo **addr) { freeaddrinfo(*addr); }
	int shutdownSocketSend(SOCKET &cSocket) {
		if (shutdown(cSocket, SD_SEND) == SOCKET_ERROR)
			return WSAGetLastError();
		return 0;
	}

	int sendRawMessage(SOCKET &sock, const char *cstrtext, int len) {
		int sent, ret;

		sent = ret = 0;
		while (sent < len) {
			ret = send(sock, cstrtext + sent, len - sent, 0);
			if (ret == SOCKET_ERROR) {
				ret = WSAGetLastError();
				return ret;
			}
			sent += ret;
		}
		return ret;
	}
	int sendRawMessage(SOCKET &sock, std::string strText) {
		return sendRawMessage(
				sock, strText.c_str(), static_cast<int>(strText.length()));
	}
	int sendRawMessage(SOCKET &sock, std::string *strText) {
		return sendRawMessage(
				sock, strText->c_str(), static_cast<int>(strText->length()));
	}
}

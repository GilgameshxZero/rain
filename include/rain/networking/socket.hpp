// A platform-agnostic OOP socket implementation.
// Throws std::system_error instead of returning error code.

#pragma once

#include "../platform.hpp"
#include "../time.hpp"
#include "native-socket.hpp"
#include "node-service-host.hpp"

#include <functional>
#include <stdexcept>

namespace Rain::Networking {
	// Not thread-safe.
	class Socket {
		public:
		// Custom types for family, type and protocol.
		enum class Family { IPV4 = AF_INET };
		enum class Type { STREAM = SOCK_STREAM, RAW = SOCK_RAW };
		enum class Protocol { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

		// Custom types for send and recv.
		enum class SendFlag {
			NONE = 0,
#ifdef RAIN_WINDOWS
			NO_SIGNAL = 0,
#else
			NO_SIGNAL = MSG_NOSIGNAL,
#endif
			NO_ROUTE = MSG_DONTROUTE,
			OUT_OF_BAND = MSG_OOB
		};
		enum class RecvFlag { NONE = 0 };

		// Sockets are created in an invalid state.
		Socket(bool create = false,
			NativeSocket nativeSocket = NATIVE_SOCKET_INVALID,
			Family family = Family::IPV4,
			Type type = Type::STREAM,
			Protocol protocol = Protocol::TCP)
				: nativeSocket(nativeSocket),
					family(family),
					type(type),
					protocol(protocol) {
			Socket::startup();

			// Unless create is specified.
			if (create) {
				this->create();
			}
		}

		// Shorthand for calling WSA stuff in Windows and doing nothing otherwise.
		static void startup() {
#ifdef RAIN_WINDOWS
			static bool calledBefore = false;
			static WSADATA wsaData;
			if (!calledBefore) {
				calledBefore = true;
				int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (status) {
					throw std::runtime_error(
						("WSAStartup failed with code " + std::to_string(status)).c_str());
				}
			}
#endif
		}
		static void cleanup() {
#ifdef RAIN_WINDOWS
			int status = WSACleanup();
			if (status) {
				throw std::runtime_error(
					("WSACleanup failed with code " + std::to_string(status)).c_str());
			}
#endif
		}

		// Initialize this socket if it is invalid, or return the current socket.
		NativeSocket create() {
			if (this->nativeSocket == NATIVE_SOCKET_INVALID) {
				this->nativeSocket = ::socket(static_cast<int>(this->family),
					static_cast<int>(this->type),
					static_cast<int>(this->protocol));
				if (this->nativeSocket == NATIVE_SOCKET_INVALID) {
					throw std::runtime_error(
						("socket failed with code " + std::to_string(this->nativeSocket))
							.c_str());
				}
			}
			return this->nativeSocket;
		}

		// Platform-agnostic socket operations adapted to the Socket interface.
		void connect(const Host &host) const {
			this->connectOrBind(
				host, [](addrinfo *) {}, ::connect);
		}
		void bind(const Host &host) const {
			this->connectOrBind(
				host, [](addrinfo *hints) { hints->ai_flags = AI_PASSIVE; }, ::bind);
		}
		void listen(int backlog = 1024) const {
			int status = ::listen(this->nativeSocket, backlog);
			if (status != 0) {
				throw std::runtime_error(
					("listen failed with code " + std::to_string(status)).c_str());
			}
		}

		// Accept uses select to block for new connections.
		// Timing out will return an invalid socket.
		NativeSocket accept(sockaddr *addr = NULL,
			socklen_t *addrLen = NULL, std::size_t timeoutMs = 0) const {
			if (this->blockForSelect(true, timeoutMs)) {
				return NATIVE_SOCKET_INVALID;
			}
			NativeSocket newSocket = ::accept(this->nativeSocket, addr, addrLen);
			if (newSocket == NATIVE_SOCKET_INVALID) {
				throw std::runtime_error(
					("accept failed with code " + std::to_string(newSocket)).c_str());
			}
			return newSocket;
		}

		// Block until send all bytes. Uses select to make sure we can write before
		// we try. Returns number of bytes sent (may be different if using timeout).
		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			if (len == 0) {
				len = strlen(msg);
			}

			std::size_t bytesSent = 0;
			auto timeoutTime =
				std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
			while (bytesSent != len) {
				if (this->blockForSelect(false,
							timeoutMs == 0
								? 0
								: std::chrono::duration_cast<std::chrono::milliseconds>(
										timeoutTime - std::chrono::steady_clock::now())
										.count())) {
					break;
				}
				int status = ::send(this->nativeSocket,
#ifdef RAIN_WINDOWS
					msg + bytesSent,
					static_cast<int>(len - bytesSent),
#else
					reinterpret_cast<const void *>(msg + bytesSent),
					len - bytesSent,
#endif
					static_cast<int>(flags));
				if (status == NATIVE_SOCKET_INVALID) {
					throw std::runtime_error(
						("send failed with code " + std::to_string(status)).c_str());
				}
				bytesSent += static_cast<std::size_t>(status);
			}
			return bytesSent;
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return this->send(s.c_str(), s.length(), flags, timeoutMs);
		}

		// Block until recv some bytes, or exception if something went wrong.
		// Uses select to block to correctly break block if socket closed.
		// Returns number of bytes received, 0 on timeout.
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::size_t timeoutMs = 0) const {
			if (this->blockForSelect(true, timeoutMs)) {
				return 0;
			}
			int status = ::recv(this->nativeSocket,
#ifdef RAIN_WINDOWS
				buf,
				static_cast<int>(len),
#else
				reinterpret_cast<void *>(buf),
				len,
#endif
				static_cast<int>(flags));
			if (status < 0) {
				throw std::runtime_error(
					("recv failed with code " + std::to_string(status)).c_str());
			} else if (status == 0) {
				throw std::runtime_error("recv closed gracefully.");
			}
			return status;
		}

		// Tries to shutdown and then closes. Returns nonzero on error. Shutdown can
		// fail silently.
		void close() {
			if (this->nativeSocket == NATIVE_SOCKET_INVALID) {
				return;
			}

#ifdef RAIN_WINDOWS
			::shutdown(this->nativeSocket, SD_BOTH);
#else
			::shutdown(this->nativeSocket, SHUT_RDWR);
#endif
			int status;
#ifdef RAIN_WINDOWS
			status = closesocket(this->nativeSocket);
#else
			status = ::close(this->nativeSocket);
#endif
			this->nativeSocket = NATIVE_SOCKET_INVALID;
			if (status == NATIVE_SOCKET_INVALID) {
				throw std::runtime_error(
					("close or closesocket failed with code " + std::to_string(status))
						.c_str());
			}
		}

		// Getters.
		NativeSocket getNativeSocket() { return this->nativeSocket; }
		Family getFamily() { return this->family; }
		Type getType() { return this->type; }
		Protocol getProtocol() { return this->protocol; }

		private:
		// Internal socket.
		NativeSocket nativeSocket;
		Family family;
		Type type;
		Protocol protocol;

		// Shared code for bind and connect.
		void connectOrBind(const Host &host,
			std::function<void(addrinfo *)> setHints,
			std::function<int(NativeSocket, sockaddr *, int)> action) const {
			addrinfo hints, *result, *curAddr;

			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = static_cast<int>(this->family);
			hints.ai_socktype = static_cast<int>(this->type);
			hints.ai_protocol = static_cast<int>(this->protocol);
			setHints(&hints);
			int status = getaddrinfo(
				host.node.getCStr(), host.service.getCStr(), &hints, &result);
			if (status != 0) {
				throw std::runtime_error(
					("getaddrinfo failed with code " + std::to_string(status)).c_str());
			}

			// Try all the addresses we found.
			curAddr = result;
			while (curAddr != NULL) {
				status = action(this->nativeSocket,
					curAddr->ai_addr,
					static_cast<int>(curAddr->ai_addrlen));
				if (status == 0) {
					return;
				}
				curAddr = curAddr->ai_next;
			}
			throw std::runtime_error(
				("connect or bind failed with code " + std::to_string(status)).c_str());
		}

		// Returns true if terminated on timeout. By default, blocks without
		// timeout.
		bool blockForSelect(bool read, std::size_t timeoutMs = 0) const {
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(this->nativeSocket, &fds);
			timeval tv;
			tv.tv_sec = static_cast<long>(timeoutMs) / 1000;
			tv.tv_usec = (timeoutMs % 1000) * 1000;
			int status = select(static_cast<int>(this->nativeSocket) + 1,
				read ? &fds : NULL,
				read ? NULL : &fds,
				&fds,
				timeoutMs == 0 ? NULL : &tv);
			if (status == 0) {	// Timeout.
				return true;
			} else if (status == NATIVE_SOCKET_INVALID) {
				// Error with select.
				throw std::runtime_error(
					("select failed with code " + std::to_string(NATIVE_SOCKET_INVALID))
						.c_str());
			}
			return false;
		}
	};
}

// On Windows, WSAStartup must be called before any socket functionality is
// used. On POSIX-like systems, these functions are no-op. Code which uses
// networking should always call this function first.
#pragma once

#include "exception.hpp"
#include "native-socket.hpp"

#include <mutex>

namespace Rain::Networking::Wsa {
	enum class Error { AT_EXIT_CLEANUP_NO_REGISTER };
	class ErrorCategory : public std::error_category {
		public:
		char const *name() const noexcept { return "Rain::Networking::Wsa"; }
		std::string message(int error) const noexcept {
			switch (static_cast<Error>(error)) {
				case Error::AT_EXIT_CLEANUP_NO_REGISTER:
					return "Failed to register WSACleanup with atexit.";
				default:
					return "Generic.";
			}
		}
	};
	typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

#ifndef RAIN_PLATFORM_WINDOWS
	typedef void WSADATA;
#endif
	// On Windows systems, calls WSAStartup for Winsock 2.2, and registers a call
	// for WSACleanup on exit. This function will only run once per application,
	// and is thread-safe. Subsequent calls return the WSADATA obtained by the
	// first run.
	WSADATA prepare() {
#ifdef RAIN_PLATFORM_WINDOWS
		// Serializes function.
		static std::mutex fncMtx;

		// This function only needs to be called once per application.
		static bool calledBefore = false;

		// WSADATA filled by WSAStartup.
		static WSADATA wsaData;

		std::lock_guard<std::mutex> fncMtxLckGuard(fncMtx);

		// This operation should be transactional, but there is no way to guarantee
		// this in Winsock.
		if (!calledBefore) {
			int startupReturn = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (startupReturn != 0) {
				// An immediate failure can throws an error code from the WSA set of
				// codes.
				throw Networking::Exception(
					static_cast<Networking::Error>(startupReturn));
			}

			if (atexit([]() {
						if (WSACleanup() == NATIVE_SOCKET_ERROR) {
							throw Networking::Exception(
								static_cast<Networking::Error>(WSAGetLastError()));
						}
					}) != 0) {
				throw Exception(Error::AT_EXIT_CLEANUP_NO_REGISTER);
			}

			calledBefore = true;
		}
		return wsaData;
#endif
	}
}

// Error category for general networking errors, obtained via errno or
// WSAGetLastError.
#pragma once

#include "../error/exception.hpp"
#include "../literal.hpp"
#include "../platform.hpp"
#include "../windows.hpp"
#include "native-socket.hpp"

namespace Rain::Networking {
	enum class Error {
		NONE = 0,

#ifdef RAIN_PLATFORM_WINDOWS
		IN_PROGRESS = WSAEINPROGRESS,
		WOULD_BLOCK = WSAEWOULDBLOCK,
		TIMED_OUT = WSAETIMEDOUT,
		NOT_CONNECTED = WSAENOTCONN,
#else
		IN_PROGRESS = EINPROGRESS,
		WOULD_BLOCK = EWOULDBLOCK,
		TIMED_OUT = ETIMEDOUT,
		NOT_CONNECTED = ENOTCONN,
#endif

		// Resolver errors are also thrown as base Networking errors.
		RES_QUERY_FAILED = 1_zu << 16,
		NS_INITPARSE_FAILED,
		NS_MSG_COUNT_FAILED,

		// Additional errors from base SocketInterface are thrown here too.
		POLL_INVALID
	};
	class ErrorCategory : public std::error_category {
		public:
		char const *name() const noexcept { return "Rain::Networking"; }
		std::string message(int error) const noexcept {
			switch (static_cast<Error>(error)) {
				case Error::RES_QUERY_FAILED:
					return "res_query failed.";
				case Error::NS_INITPARSE_FAILED:
					return "ns_initparse failed.";
				case Error::NS_MSG_COUNT_FAILED:
					return "ns_msg_count failed.";
				case Error::POLL_INVALID:
					return "poll on invalid socket.";
				default:
					break;
			}

#ifdef RAIN_PLATFORM_WINDOWS
			// While Rain builds with UNICODE, we call the ANSI functions here, since
			// error messages are not to be wide strings. Failure should always free
			// buffer if it is successfully allocated.
			LPSTR buffer;
			if (
				FormatMessageA(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					static_cast<DWORD>(error),
					// User default language.
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					reinterpret_cast<LPSTR>(&buffer),
					0,
					nullptr) == 0) {
				// If FormatMessage fails, don't attempt to interpret the error anymore.
				return "FormatMessage failed while retrieving message.";
			}

			std::string msg(buffer);
			if (LocalFree(buffer) != nullptr) {
				return "LocalFree failed while retrieving message. Original message: " +
					msg;
			}

			// Messages are not to be newline-terminated.
			if (msg.length() > 0 && msg.back() == '\n') {
				msg.pop_back();
			}
			return msg;
#else
			return std::strerror(error);
#endif
		}
	};
	typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

	// Return a system error code via errno or WSAGetLastError.
	inline Error getSystemError() noexcept {
#ifdef RAIN_PLATFORM_WINDOWS
		return static_cast<Error>(WSAGetLastError());
#else
		return static_cast<Error>(errno);
#endif
	}

	// Helper function throws a system exception if return value is an error
	// value.
	inline NativeSocket validateSystemCall(NativeSocket result) {
		if (result == NATIVE_SOCKET_INVALID) {
			throw Exception(getSystemError());
		}
		return result;
	}
#ifdef RAIN_PLATFORM_WINDOWS
	inline int validateSystemCall(int result) {
		if (result == NATIVE_SOCKET_ERROR) {
			throw Exception(getSystemError());
		}
		return result;
	}
#endif
}

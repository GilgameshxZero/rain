// Error category for general networking errors, obtained via errno or
// WSAGetLastError.
#pragma once

#include "../error/exception.hpp"
#include "../literal.hpp"
#include "../platform.hpp"
#include "../windows/windows.hpp"
#include "../windows/exception.hpp"
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
			return Windows::ErrorCategory::getLastErrorMessage(error);
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

// Error category for general networking errors, obtained via errno or
// WSAGetLastError.
#pragma once

#include "../error/exception.hpp"
#include "../platform.hpp"
#include "../windows.hpp"
#include "native-socket.hpp"

namespace Rain::Networking {
	enum class Error : int {
#ifdef RAIN_PLATFORM_WINDOWS
		IN_PROGRESS = WSAEINPROGRESS,
		WOULD_BLOCK = WSAEWOULDBLOCK,
		NOT_CONNECTED = WSAENOTCONN
#else
		IN_PROGRESS = EINPROGRESS,
		WOULD_BLOCK = EWOULDBLOCK,
		NOT_CONNECTED = ENOTCONN
#endif
	};
	class ErrorCategory : public std::error_category {
		public:
		char const *name() const noexcept { return "Rain::Networking"; }
		std::string message(int error) const noexcept {
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
}
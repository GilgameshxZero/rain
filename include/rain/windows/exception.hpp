// Common error-handling and exceptions for GetLastError-like errors.
#pragma once

#include "../error/exception.hpp"
#include "windows.hpp"

namespace Rain::Windows {
	enum class Error { NONE = 0 };
	class ErrorCategory : public std::error_category {
		public:
		// Shared code to translate a value from GetLastError/WSAGetLastError into a
		// meaningful message.
		static std::string getLastErrorMessage(int error) noexcept {
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
			return "Attempted to retrieve Windows exception message on non-Windows "
						 "platform.";
#endif
		}

		char const *name() const noexcept { return "Rain::Windows"; }
		std::string message(int error) const noexcept {
			return ErrorCategory::getLastErrorMessage(error);
		}
	};
	typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

	// Return a system error code via errno or WSAGetLastError.
	inline Error getSystemError() noexcept {
#ifdef RAIN_PLATFORM_WINDOWS
		return static_cast<Error>(GetLastError());
#else
		return Error::NONE;
#endif
	}

	// Helper function throws a system exception if return value is an error
	// value. Most Windows system calls return NULL or 0 on failure.
	template <typename Result>
	inline auto validateSystemCall(Result &&result) {
		if (result == NULL) {
			throw Exception(getSystemError());
		}
		return result;
	}
}

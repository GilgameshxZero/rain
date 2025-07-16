// Exception handling for Gdiplus.
#pragma once

#include "../../error/exception.hpp"
#include "gdiplus.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

namespace Rain::Windows::Gdiplus {
	using Error = ::Gdiplus::Status;
	class ErrorCategory : public std::error_category {
		public:
		char const *name() const noexcept { return "Rain::Windows::Gdiplus"; }
		std::string message(int error) const noexcept {
			switch (error) {
				case ::Gdiplus::Status::Ok:
					return "Ok.";
				default:
					return "Please look up the error for more details.";
			}
		}
	};
	typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

	// Gdiplus functions return Gdiplus::Status::Ok on success.
	inline void validateGdiplusCall(::Gdiplus::Status result) {
		if (result != ::Gdiplus::Status::Ok) {
			throw Exception(result);
		}
	}
}

#endif

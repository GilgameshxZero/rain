// Errors are generic error codes and can be returned. Exceptions are error
// codes which indicate critical failure and shall be thrown.
#pragma once

#include "string.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>

namespace Rain {
	class Exception : public std::exception {
		public:
		const char *what() const noexcept { return this->whatStr.c_str(); }
		Exception(const std::error_condition &ec) {
			this->whatStr = std::string(ec.category().name()) + " (" +
				std::to_string(ec.value()) + "): " + ec.category().message(ec.value());
		}

		private:
		std::string whatStr;
	};
}

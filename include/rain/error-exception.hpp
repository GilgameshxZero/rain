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
		Exception(const std::error_condition &ec)
				: ec(ec),
					whatStr(std::string(this->ec.category().name()) + " (" +
						std::to_string(this->ec.value()) +
						"): " + this->ec.category().message(this->ec.value())) {}

		const char *what() const noexcept { return this->whatStr.c_str(); }
		const std::error_condition getErrorCondition() { return this->ec; }

		private:
		const std::error_condition ec;
		const std::string whatStr;
	};
}

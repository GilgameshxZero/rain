/*
Errors are generic error codes and can be returned.Exceptions are error codes
which indicate critical failure and shall be thrown.

To be used with custom std::error_condition and codes, such as those defined in
`rain/networking/socket.hpp`.
*/

#pragma once

#include "../string.hpp"

#include <cerrno>
#include <stdexcept>
#include <system_error>

namespace Rain::ErrorException {
	class Exception : public std::exception {
		public:
		Exception(std::error_condition const &ec)
				: ec(ec),
					whatStr(std::string(this->ec.category().name()) + " (" +
						std::to_string(this->ec.value()) +
						"): " + this->ec.category().message(this->ec.value())) {}

		char const *what() const noexcept { return this->whatStr.c_str(); }
		std::error_condition const getErrorCondition() { return this->ec; }

		private:
		std::error_condition const ec;
		std::string const whatStr;

		// Utility functions for classes using Exception to use. See
		// `rain/string/command-line-parser.hpp` for an example/
		public:
		template <typename ErrorCategoryType, typename ErrorType>
		static ErrorException::Exception makeException(ErrorType e) {
			static ErrorCategoryType errorCategory;
			return ErrorException::Exception(
				std::error_condition(static_cast<int>(e), errorCategory));
		}
	};
}

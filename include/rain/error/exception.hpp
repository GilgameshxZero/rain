// Provides an interface for declaring custom exceptions.
//
// Exceptions take an enum class Error, representing the error code, and an
// ErrorCategory subclassing std::error_category, handling translating the error
// code into an error message.
#pragma once

#include "../string/string.hpp"

#include <stdexcept>
#include <system_error>

namespace Rain::Error {
	// Subclasses std::exception, implements custom what() string to display error
	// category name, code, and error code message.
	//
	// Default template arguments provided for ease of use in catch blocks.
	template <typename Error, typename ErrorCategory>
	class Exception : public std::exception {
		private:
		// A unique ErrorCategory instance.
		static inline ErrorCategory const errorCategory;

		Error const error;
		std::string const explanation;

		public:
		// Construct an exception with just the error code, in the format "CATEGORY,
		// ERROR: MESSAGE".
		Exception(Error const &error = static_cast<Error>(0))
				: error(error),
					explanation(
						std::string(errorCategory.name()) + ", " +
						std::to_string(static_cast<int>(error)) + ": " +
						errorCategory.message(static_cast<int>(error)) + "\n") {}

		// Return the error (code).
		Error const &getError() const noexcept { return this->error; }

		// Return the ErrorCategory for checking equality.
		static ErrorCategory const &getErrorCategory() noexcept {
			return Exception<Error, ErrorCategory>::errorCategory;
		}

		// Return the preconstructed explanation string.
		//
		// Because std::exception::what is virtual, catch blocks only have to catch
		// std::exception to use this version of what.
		char const *what() const noexcept { return this->explanation.c_str(); }
	};

	// Default generic error category.
	class GenericErrorCategory : public std::error_category {
		public:
		// Name of this category of errors.
		char const *name() const noexcept { return "Generic Error Category"; }

		// Translates Errors from the enum into string messages.
		std::string message(int error) const noexcept { return "Generic."; }
	};
	using GenericException = Exception<int, GenericErrorCategory>;
}

// Tests for Rain::Error::Exception.
#include <rain/error/exception.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>

// Defines platform-independent codes.
enum class Error {
	NONE = 0,

	// Example errors.
	ERROR_1,
	ERROR_2
};

// ErrorCategory classes are required to implement name and message.
class ErrorCategory : public std::error_category {
	public:
	// Name of this category of errors.
	char const *name() const noexcept { return "Error Category"; }

	// Translates Errors from the enum into string messages.
	std::string message(int error) const noexcept {
		switch (static_cast<Error>(error)) {
			case Error::NONE:
				return "None.";
			case Error::ERROR_1:
				return "Error message for error 1.";
			case Error::ERROR_2:
				return "Error message for error 2.";
			default:
				return "Generic.";
		}
	}
};

// The custom exception class is simply defined from Error and ErrorCategory.
typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

int main() {
	// Throw and catch an exceptions.
	{
		std::cout << "Throwing error..." << std::endl;
		try {
			throw Exception(Error::ERROR_1);

			throw std::runtime_error("Did not throw an error where expected.");
		} catch (Exception const &exception) {
			std::cout << exception.what();
			assert(
				strcmp(
					exception.what(),
					"Error Category, 1: Error message for error 1.\n") == 0);
		}
		std::cout << std::endl;
	}

	return 0;
}

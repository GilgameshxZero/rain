// Tests for Rain::Error::consumeThrowable.
#include <rain/error/consume-throwable.hpp>

#include <rain/literal.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
	// Wrap a non-throwing callable and execute it.
	{
		Rain::Error::consumeThrowable(
			[]() { std::cout << "This function should not throw." << std::endl; },
			RAIN_ERROR_LOCATION)();
	}

	// Wrap a throwing callable and execute it.
	{
		Rain::Error::consumeThrowable(
			[]() {
				std::cout << "This function will throw." << std::endl;
				throw std::exception();
			},
			RAIN_ERROR_LOCATION)();
	}

	// Consume a throwable with captures and parameters.
	{
		int left = 3;
		assert(
			Rain::Error::consumeThrowable<int>(
				[left](int right) {
					std::cout << "This function computes the sum " << left + right << "."
										<< std::endl;

					return left + right;
				},
				RAIN_ERROR_LOCATION)(5) == 8);
	}

	// Throw and ensure that managed type is default-constructed on return.
	{
		using namespace Rain::Literal;

		// Callable must not return default-constructed C-Strings to ensure their
		// validity when passed to the std::string constructor.
		auto callable = [](bool shouldThrow) {
			std::cout
				<< "This function returns a string, optionally throwing beforehand."
				<< std::endl;

			if (shouldThrow) {
				throw std::exception();
			}

			return "a string"s;
		};
		auto consumedCallable =
			Rain::Error::consumeThrowable<bool>(callable, RAIN_ERROR_LOCATION);
		std::string noThrowStatus(consumedCallable(false)),
			throwStatus(consumedCallable(true));
		std::cout << noThrowStatus << ", " << throwStatus << std::endl;
		assert(noThrowStatus == "a string");
		assert(throwStatus.empty());
	}

	// References should be forwarded through consumeThrowable.
	{
		int value = 0;

		Rain::Error::consumeThrowable<int &>(
			[](int &value) { value++; }, RAIN_ERROR_LOCATION)(value);
		assert(value == 1);
	}

	return 0;
}

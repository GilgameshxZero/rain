// Tests for Rain::Error::consumeThrowable.
#include <rain/error/consume-throwable.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>

auto foo() {
	static int count = 0;
	return count++;
}

auto bar(int val) {
	return val;
}
std::string bar() {
	return "hi!";
}

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
		left++;
		assert(
			Rain::Error::consumeThrowable(
				[left](int right) {
					std::cout << "This function computes the sum " << left + right << "."
										<< std::endl;

					return left + right;
				},
				RAIN_ERROR_LOCATION)(5) == 9);
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
			Rain::Error::consumeThrowable(callable, RAIN_ERROR_LOCATION);
		std::string noThrowStatus(consumedCallable(false)),
			throwStatus(consumedCallable(true));
		std::cout << noThrowStatus << ", " << throwStatus << std::endl;
		assert(noThrowStatus == "a string");
		assert(throwStatus.empty());
	}

	// References should be forwarded through consumeThrowable.
	{
		int value = 0;

		Rain::Error::consumeThrowable(
			[](int &value) { value++; }, RAIN_ERROR_LOCATION)(value);
		assert(value == 1);
	}

	// Mutable lambdas must be handled as well.
	{
		int value = 0;

		auto throwable = [value]() mutable {
			// This lambda must be marked mutable since copy-capture value is
			// modified.
			value += 1;
			return value;
		};
		assert(throwable() == 1);
		assert(throwable() == 2);
		auto consumed = Rain::Error::consumeThrowable(throwable);
		assert(consumed() == 3);
		assert(consumed() == 4);

		// std::move on lambda copies it.
		assert(throwable() == 3);
	}

	// Function pointers should behave normally (i.e. copied during move).
	{
		auto consumed = Rain::Error::consumeThrowable(foo);
		assert(foo() == 0);
		assert(consumed() == 1);
		assert(foo() == 2);
		assert(consumed() == 3);
	}

	// std::function MAY OR MAY NOT be moved depending on implementation.
	{
		std::function<int()> f([]() {
			static int value = 0;
			return value++;
		});

		assert(f() == 0);
		auto consumed = Rain::Error::consumeThrowable(std::move(f));
		try {
			assert(f() == 1);
		} catch (std::exception const &exception) {
			std::cout << exception.what();
		}
		consumed();
	}

	// Function pointer overloads.
	{
		auto consumed =
			Rain::Error::consumeThrowable(RAIN_FUNCTIONAL_RESOLVE_OVERLOAD(bar));
		assert(consumed() == "hi!");
		assert(consumed(3) == 3);
		assert(consumed(19) == 19);
		assert(consumed() == "hi!");
	}

	return 0;
}

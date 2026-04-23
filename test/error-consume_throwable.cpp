// Tests for Rain::Error::consumeThrowable.
#include <rain.hpp>

using Rain::Error::releaseAssert;

auto foo() {
	static int count{0};
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
			[]() {
				std::cout << "This function should not throw."
									<< std::endl;
			},
			std::source_location::current())();
	}

	// Wrap a throwing callable and execute it.
	{
		Rain::Error::consumeThrowable(
			[]() {
				std::cout << "This function will throw."
									<< std::endl;
				throw std::exception();
			},
			std::source_location::current())();
	}

	// Consume a throwable with captures and parameters.
	{
		int left{3};
		left++;
		releaseAssert(
			Rain::Error::consumeThrowable(
				[left](int right) {
					std::cout << "This function computes the sum "
										<< left + right << "." << std::endl;

					return left + right;
				},
				std::source_location::current())(5) == 9);
	}

	// Throw and ensure that managed type is
	// default-constructed on return.
	{
		using namespace Rain::Literal;

		// Callable must not return default-constructed
		// C-Strings to ensure their validity when passed to the
		// std::string constructor.
		auto callable = [](bool shouldThrow) {
			std::cout << "This function returns a string, "
									 "optionally throwing beforehand."
								<< std::endl;

			if (shouldThrow) {
				throw std::exception();
			}

			return "a string"s;
		};
		auto consumedCallable = Rain::Error::consumeThrowable(
			callable, std::source_location::current());
		std::string noThrowStatus(consumedCallable(false)),
			throwStatus(consumedCallable(true));
		std::cout << noThrowStatus << ", " << throwStatus
							<< std::endl;
		releaseAssert(noThrowStatus == "a string");
		releaseAssert(throwStatus.empty());
	}

	// References should be forwarded through
	// consumeThrowable.
	{
		int value{0};

		Rain::Error::consumeThrowable(
			[](int &value) { value++; },
			std::source_location::current())(value);
		releaseAssert(value == 1);
	}

	// Mutable lambdas must be handled as well.
	{
		int value{0};

		auto throwable = [value]() mutable {
			// This lambda must be marked mutable since
			// copy-capture value is modified.
			value += 1;
			return value;
		};
		releaseAssert(throwable() == 1);
		releaseAssert(throwable() == 2);
		auto consumed =
			Rain::Error::consumeThrowable(throwable);
		releaseAssert(consumed() == 3);
		releaseAssert(consumed() == 4);

		// std::move on lambda copies it.
		releaseAssert(throwable() == 3);
	}

	// Function pointers should behave normally (i.e. copied
	// during move).
	{
		auto consumed = Rain::Error::consumeThrowable(foo);
		releaseAssert(foo() == 0);
		releaseAssert(consumed() == 1);
		releaseAssert(foo() == 2);
		releaseAssert(consumed() == 3);
	}

	// std::function MAY OR MAY NOT be moved depending on
	// implementation.
	{
		std::function<int()> f([]() {
			static int value{0};
			return value++;
		});

		releaseAssert(f() == 0);
		auto consumed =
			Rain::Error::consumeThrowable(std::move(f));
		try {
			releaseAssert(f() == 1);
		} catch (std::exception const &exception) {
			std::cout << exception.what();
		}
		consumed();
	}

	// Function pointer overloads.
	{
		auto consumed = Rain::Error::consumeThrowable(
			RAIN_FUNCTIONAL_RESOLVE_OVERLOAD(bar));
		releaseAssert(consumed() == "hi!");
		releaseAssert(consumed(3) == 3);
		releaseAssert(consumed(19) == 19);
		releaseAssert(consumed() == "hi!");
	}

	return 0;
}

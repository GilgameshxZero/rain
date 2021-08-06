// Tests for Time::Timeout.
#include <rain/literal.hpp>
#include <rain/time/time.hpp>

#include <rain/time/timeout.hpp>

#include <cassert>
#include <iostream>
#include <thread>

using namespace Rain::Literal;

// Sleep for some time since the time of execution using sleep_until.
template <typename Clock = std::chrono::steady_clock>
void sleep(
	// Default to sleeping for 3s.
	//
	// Okay to use const reference, since the Timeout constructor is evaluated at
	// execution.
	Rain::Time::Timeout<Clock> const &timeout = 3s) {
	// May block indefinitely on faulty implementations of sleep_until with passed
	// time.
	std::this_thread::sleep_until(timeout.getTimeoutTime());
}

int main() {
	// Sleep for 1s using sleep_for directly.
	{
		auto timeBegin = std::chrono::steady_clock::now();

		std::cout << "Sleeping for 1s..." << std::endl;
		Rain::Time::Timeout timeout(1s);
		std::this_thread::sleep_for(timeout.getTimeUntilTimeout());

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed > 0.9s && timeElapsed < 1.1s);
		assert(timeout.hasPassed());
	}

	// Sleep for 1s using sleep_until directly.
	{
		auto timeBegin = std::chrono::steady_clock::now();

		std::cout << "Sleeping for 1s..." << std::endl;
		Rain::Time::Timeout timeout(1s);
		std::this_thread::sleep_until(timeout.getTimeoutTime());

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed > 0.9s && timeElapsed < 1.1s);
		assert(timeout.hasPassed());
	}

	// Default timeout arguments declared as const references should be evaluated
	// at execution as expected.
	{
		auto timeBegin = std::chrono::steady_clock::now();

		std::cout << "Sleeping for 3s..." << std::endl;
		sleep();
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed > 2.9s && timeElapsed < 3.1s);
	}

	// Timeout should be able to be constructed inline using list-initialization.
	{
		auto timeBegin = std::chrono::steady_clock::now();

		std::cout << "Sleeping for 2s..." << std::endl;
		sleep({2s});
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed > 1.9s && timeElapsed < 2.1s);
	}

	// Default-constructed timeouts expire immediately.
	{
		auto timeBegin = std::chrono::steady_clock::now();

		std::cout << "Waking immediately..." << std::endl;
		sleep({0s});
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed < 0.1s);
	}

	return 0;
}

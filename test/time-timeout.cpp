// Tests for Time::Timeout.
#include <rain.hpp>

using Rain::Error::releaseAssert;
using namespace Rain::Literal;

// Sleep for some time since the time of execution using
// sleep_until.
void sleep(
	// Default to sleeping for 300ms.
	//
	// Okay to use const reference, since the Timeout
	// constructor is evaluated at execution.
	Rain::Time::Timeout const &timeout = 300ms) {
	// May block indefinitely on faulty implementations of
	// sleep_until with passed time.
	std::this_thread::sleep_until(timeout.asTimepoint());
}

int main() {
	// Sleep for 100ms using sleep_for directly.
	{
		std::cout << "Sleeping for 100ms..." << std::endl;
		Rain::Time::Timeout timeout(100ms);
		auto timeBegin = std::chrono::steady_clock::now();
		std::this_thread::sleep_for(timeout.asDuration());
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl
							<< std::endl;
		releaseAssert(
			timeElapsed > 80ms && timeElapsed < 120ms);
		releaseAssert(timeout.isPassed());
	}

	// Sleep for 1s using sleep_until directly.
	{
		std::cout << "Sleeping for 100ms..." << std::endl;
		Rain::Time::Timeout timeout(100ms);
		auto timeBegin = std::chrono::steady_clock::now();
		std::this_thread::sleep_until(timeout.asTimepoint());
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl
							<< std::endl;
		releaseAssert(
			timeElapsed > 80ms && timeElapsed < 120ms);
		releaseAssert(timeout.isPassed());
	}

	// Default timeout arguments declared as const references
	// should be evaluated at execution as expected.
	{
		std::cout << "Sleeping for 300ms..." << std::endl;
		auto timeBegin = std::chrono::steady_clock::now();
		sleep();
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl
							<< std::endl;
		releaseAssert(
			timeElapsed > 240ms && timeElapsed < 360ms);
	}

	// Timeout should be able to be constructed inline using
	// list-initialization.
	{
		std::cout << "Sleeping for 200ms..." << std::endl;
		auto timeBegin = std::chrono::steady_clock::now();
		sleep({200ms});
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl
							<< std::endl;
		releaseAssert(
			timeElapsed > 160ms && timeElapsed < 240ms);
	}

	// Default-constructed timeouts expire immediately.
	{
		std::cout << "Waking immediately..." << std::endl;
		auto timeBegin = std::chrono::steady_clock::now();
		sleep({0s});
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time passed: " << timeElapsed << std::endl
							<< std::endl;
		releaseAssert(timeElapsed < 20ms);
	}

	return 0;
}

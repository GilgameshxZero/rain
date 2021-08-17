// Represents a timeout event on std::chrono::steady_clock. Can either be an
// event in the future, an event that has already passed, or an event which
// never occurs.
#pragma once

#include <chrono>

namespace Rain::Time {
	// Represents a timeout event on std::chrono::steady_clock. Can either be an
	// event in the future, an event that has already passed, or an event which
	// never occurs.
	class Timeout {
		private:
		// Timeout is always represented as a time_point.
		// If this is in the future, timeout has yet to occur. If this is in the
		// past or present, timeout has already occurred. If this is
		// Clock::time_point::max, there is no timeout.
		std::chrono::steady_clock::time_point timeoutTimepoint;

		public:
		// Construct with time_point. By default, infinite timeout.
		Timeout(
			std::chrono::steady_clock::time_point timeoutTimepoint =
				std::chrono::steady_clock::time_point::max())
				: timeoutTimepoint(timeoutTimepoint) {}

		// Construct with duration.
		template <typename Rep, typename Period>
		Timeout(std::chrono::duration<Rep, Period> durationUntilTimeout)
				: Timeout(std::chrono::steady_clock::now() + durationUntilTimeout) {}

		bool isInfinite() const noexcept {
			return this->timeoutTimepoint ==
				std::chrono::steady_clock::time_point::max();
		}
		bool isPassed() const noexcept {
			return this->timeoutTimepoint <= std::chrono::steady_clock::now();
		}

		std::chrono::steady_clock::time_point asTimepoint() const noexcept {
			return this->timeoutTimepoint;
		}
		std::chrono::steady_clock::duration asDuration() const noexcept {
			return this->timeoutTimepoint - std::chrono::steady_clock::now();
		}
		// POSIX functions treat -1 as infinite timeout, 0 as an immediate timeout,
		// and otherwise the int as a duration in ms until timeout.
		int asInt() const noexcept {
			if (this->isInfinite()) {
				return -1;
			} else if (this->isPassed()) {
				return 0;
			} else {
				return static_cast<int>(
					std::chrono::duration_cast<std::chrono::milliseconds>(
						this->asDuration())
						.count());
			}
		}

		// Conversion operators.
		explicit operator bool() const noexcept { return this->isPassed(); }
		operator std::chrono::steady_clock::time_point() const noexcept {
			return this->asTimepoint();
		}
		operator std::chrono::steady_clock::duration() const noexcept {
			return this->asDuration();
		}
		explicit operator int() const noexcept { return this->asInt(); }

		// Sparse subset of comparison operators.
		bool operator<(Timeout const &other) const noexcept {
			return this->timeoutTimepoint < other.timeoutTimepoint;
		}
		bool operator==(Timeout const &other) const noexcept {
			return this->timeoutTimepoint == other.timeoutTimepoint;
		}
	};
}

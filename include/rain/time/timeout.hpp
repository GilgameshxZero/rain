// Represents a timeout event specification: either a time_point in the future,
// a duration, or infinity (no timeout).
#pragma once

#include <chrono>

namespace Rain::Time {
	template <typename Clock = std::chrono::steady_clock>
	class Timeout {
		private:
		// Timeout is always represented as a time_point.
		// If this is in the future, timeout has yet to occur. If this is in the
		// past or present, timeout has already occurred. If this is
		// Clock::time_point::min, there is no timeout.
		typename Clock::time_point timeoutTime;

		public:
		// Construct with time_point. By default, no timeout.
		template <
			typename TickRep = typename Clock::rep,
			typename TickPeriod = typename Clock::period>
		Timeout(
			std::chrono::time_point<Clock, std::chrono::duration<TickRep, TickPeriod>>
				timeoutTime = Clock::time_point::max())
				: timeoutTime(timeoutTime) {}

		// Construct with duration.
		template <
			typename TickRep = typename Clock::rep,
			typename TickPeriod = typename Clock::period>
		Timeout(std::chrono::duration<TickRep, TickPeriod> timeUntilTimeout)
				: timeoutTime(Clock::now() + timeUntilTimeout) {}

		// Getters.
		bool isInfinite() const noexcept {
			return this->timeoutTime == Clock::time_point::max();
		}
		typename Clock::time_point getTimeoutTime() const noexcept {
			return this->timeoutTime;
		}
		typename Clock::duration getTimeUntilTimeout() const noexcept {
			return this->timeoutTime - Clock::now();
		}
		bool hasPassed() const noexcept {
			if (this->isInfinite()) {
				return false;
			} else {
				return this->timeoutTime <= Clock::now();
			}
		}

		// Converts the timeout event into a duration compatible with some posix
		// functions. A negative value specifies an infinite timeout. 0 is a timeout
		// already passed. Otherwise, timeout is represented as a duration in ms.
		int asClassicInt() const noexcept {
			if (this->isInfinite()) {
				return -1;
			} else if (this->hasPassed()) {
				return 0;
			} else {
				return static_cast<int>(
					std::chrono::duration_cast<std::chrono::milliseconds>(
						this->getTimeUntilTimeout())
						.count());
			}
		}

		// Conversion operators.
		operator typename Clock::time_point() const noexcept {
			return this->getTimeoutTime();
		}
		operator typename Clock::duration() const noexcept {
			return this->getTimeUntilTimeout();
		}
		explicit operator int() const noexcept { return this->asClassicInt(); }

		// Comparison operators.
		bool operator<(Timeout const &other) const noexcept {
			return this->timeoutTime < other.timeoutTime;
		}
		bool operator==(Timeout const &other) const noexcept {
			return this->timeoutTime == other.timeoutTime;
		}
		bool operator>(Timeout const &other) const noexcept {
			return this->timeoutTime > other.timeoutTime;
		}
	};
}

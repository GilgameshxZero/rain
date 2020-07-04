/*
Condition variable implementation.
*/

#pragma once

#include <condition_variable>
#include <functional>

namespace Rain {
	// Wrapper class around std::condition variable safe against spurious wakeups.
	class ConditionVariable : public std::condition_variable {
		public:
		ConditionVariable() { std::condition_variable(); }

		void wait(std::unique_lock<std::mutex> &lck) {
			std::condition_variable::wait(
				lck, std::bind(&ConditionVariable::predicate, this));
		}

		template <class Rep, class Period>
		bool wait_for(std::unique_lock<std::mutex> &lck,
			const std::chrono::duration<Rep, Period> &rel_time) {
			return std::condition_variable::wait_for(
				lck, rel_time, std::bind(&ConditionVariable::predicate, this));
		}

		template <class Clock, class Duration>
		bool wait_until(std::unique_lock<std::mutex> &lck,
			const std::chrono::time_point<Clock, Duration> &abs_time) {
			return std::condition_variable::wait_for(
				lck, abs_time, std::bind(&ConditionVariable::predicate, this));
		}

		// Only makes sense to call this before notify_all.
		void notify_one() noexcept {
			this->wakesLeft++;
			std::condition_variable::notify_one();
		}

		void notify_all() noexcept {
			this->wakesLeft = -1;
			std::condition_variable::notify_all();
		}

		// Reset a notify_all call after the waiters have processed it.
		void unNotifyAll() { this->wakesLeft = 0; }

		std::mutex &getMutex() { return m; }

		private:
		int wakesLeft = 0;	// -1 means to wake infinite threads.
		std::mutex m;

		bool predicate() {
			if (this->wakesLeft == -1) {
				return true;
			} else if (this->wakesLeft > 0) {
				this->wakesLeft--;
				return true;
			} else {
				return false;
			}
		}
	};
}

/*
Standard
*/

#pragma once

#include <condition_variable>
#include <functional>

namespace Rain {
	// wrapper class around std::condition variable safe against spurious wakeups
	class ConditionVariable : public std::condition_variable {
	 public:
		ConditionVariable();

		// overrides
		void wait(std::unique_lock<std::mutex> &lck);

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

		// makes no sense to call this after notify_all
		void notify_one() noexcept;

		void notify_all() noexcept;

		// no longer wakes threads
		void unNotifyAll();

		// get mutex for unique lock
		std::mutex &getMutex();

	 private:
		int wakesLeft;	//-1 means to wake infinite threads
		std::mutex m;

		bool predicate();
	};
}

namespace Rain {
	ConditionVariable::ConditionVariable() {
		std::condition_variable();

		wakesLeft = 0;
	}

	// overrides
	void ConditionVariable::wait(std::unique_lock<std::mutex> &lck) {
		std::condition_variable::wait(
				lck, std::bind(&ConditionVariable::predicate, this));
	}

	void ConditionVariable::notify_one() noexcept {
		wakesLeft++;
		std::condition_variable::notify_one();
	}

	void ConditionVariable::notify_all() noexcept {
		wakesLeft = -1;
		std::condition_variable::notify_all();
	}

	void ConditionVariable::unNotifyAll() { wakesLeft = 0; }

	std::mutex &ConditionVariable::getMutex() { return m; }

	bool ConditionVariable::predicate() {
		if (wakesLeft == -1) {
			return true;
		} else if (wakesLeft > 0) {
			wakesLeft--;
			return true;
		} else {
			return false;
		}
	}
}

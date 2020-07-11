#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>

namespace Rain::Thread {
	// Wrapper class around std::condition variable safe against spurious wakeups.
	// Increments and decrements need to be locked as they are not atomic.
	class ConditionVariable : private std::condition_variable {
		public:
		ConditionVariable() : std::condition_variable() {}

		void wait(std::unique_lock<std::mutex> &lck) {
			std::condition_variable::wait(
				lck, std::bind(&ConditionVariable::predicate, this));
		}

		template <class Rep, class Period>
		bool waitFor(std::unique_lock<std::mutex> &lck,
			const std::chrono::duration<Rep, Period> &rel_time) {
			return this->wait_for(
				lck, rel_time, std::bind(&ConditionVariable::predicate, this));
		}

		template <class Clock, class Duration>
		bool waitUntil(std::unique_lock<std::mutex> &lck,
			const std::chrono::time_point<Clock, Duration> &abs_time) {
			return this->wait_for(
				lck, abs_time, std::bind(&ConditionVariable::predicate, this));
		}

		void notifyOne() {
			this->notifiesMtx.lock();
			this->notifies++;
			this->notify_one();
			this->notifiesMtx.unlock();
		}

		void notifyAll() {
			this->notifiesMtx.lock();
			this->allNotified = true;
			this->notify_all();
			this->notifiesMtx.unlock();
		}

		// Reset a notifyAll call after the waiters have processed it.
		void reset() {
			this->notifiesMtx.lock();
			this->allNotified = false;
			this->notifies = 0;
			this->notifiesMtx.unlock();
		}

		std::mutex &getMtx() { return this->mtx; }

		private:
		size_t notifies = 0;
		std::mutex notifiesMtx;
		bool allNotified = false;

		std::mutex mtx;

		// Implements spurious wakeup checking.
		bool predicate() {
			if (this->allNotified) {
				return true;
			}

			if (this->notifies == 0) {
				return false;
			}

			this->notifiesMtx.lock();
			this->notifies--;
			this->notifiesMtx.unlock();
			return true;
		}
	};
}

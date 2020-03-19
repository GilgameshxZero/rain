/*
Standard
*/

#pragma once

#include "condition-variable.hpp"
#include "utility-time.hpp"

namespace Rain {
	class Timer {
	 public:
		Timer();

		// manage time left in timer
		void addTime(unsigned int ms);
		unsigned int getTime();

		// setTime can only increase time, not decrease it
		void setTime(unsigned int ms);

		// wait until timer expires
		void wait();

	 private:
		ConditionVariable cv;
		unsigned int timeLeft;
	};
}

namespace Rain {
	Timer::Timer() {}

	void Timer::setTime(unsigned int ms) {
		if (ms > timeLeft) {
			addTime(ms - timeLeft);
		}
	}
	void Timer::addTime(unsigned int ms) {
		if (timeLeft == 0) {
			std::thread([&]() {
				while (timeLeft > 0) {
					unsigned int to = timeLeft;
					sleep(to);
					timeLeft -= to;
				}
				cv.notify_one();
			})
					.detach();
		}

		timeLeft += ms;
	}
	unsigned int Timer::getTime() { return timeLeft; }
	void Timer::wait() {
		std::unique_lock<std::mutex> lck(cv.getMutex());
		cv.wait(lck);
	}
}

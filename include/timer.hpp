/*
A lightweight timer implementation.
*/

#pragma once

#include "condition-variable.hpp"
#include "time.hpp"

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

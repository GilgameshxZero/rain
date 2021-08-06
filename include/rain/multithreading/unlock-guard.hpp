// Utilities for implementing the RAII pattern: unlock on construction, lock on
// destruction.
#pragma once

#include <mutex>

namespace Rain::Multithreading {
	// Like lock_guard, but unlocks on construction and locks on destruction.
	template <typename Mutex>
	class UnlockGuard {
		private:
		Mutex &mtx;

		public:
		UnlockGuard(Mutex &mtx) : mtx(mtx) { mtx.unlock(); }
		UnlockGuard(UnlockGuard const &other) = delete;
		UnlockGuard &operator=(UnlockGuard const &) = delete;
		~UnlockGuard() { this->mtx.lock(); }

		// Move constructors are not implicitly defined.
	};
}

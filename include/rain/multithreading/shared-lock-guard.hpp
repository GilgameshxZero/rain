// Implements RAII lock then unlock of a shared mutex.
#pragma once

#include <mutex>

namespace Rain::Multithreading {
	// Like lock_guard, but unlocks on construction and locks on destruction.
	template <typename Mutex>
	class SharedLockGuard {
		private:
		Mutex &mtx;

		public:
		SharedLockGuard(Mutex &mtx) : mtx(mtx) { mtx.lock_shared(); }
		SharedLockGuard(SharedLockGuard const &other) = delete;
		SharedLockGuard &operator=(SharedLockGuard const &) = delete;
		~SharedLockGuard() { this->mtx.unlock_shared(); }

		// Move constructors are not implicitly defined.
	};
}

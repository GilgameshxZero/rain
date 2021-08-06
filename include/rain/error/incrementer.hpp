// RAII class which increments on constructions and decrements on destruction,
// used with exception-safe code.
#pragma once

#include <utility>

namespace Rain::Error {
	// RAII class which increments on constructions and decrements on destruction,
	// used with exception-safe coding. Wraps an externally-allocated counter,
	// which must exist for the duration of this object.
	template <typename Counter>
	class Incrementer {
		private:
		// Internal pointer to external counter.
		Counter &counter;

		public:
		// Incrementing constructor.
		Incrementer(Counter &counter) noexcept : counter(counter) { counter++; }

		// Decrementing destructor if incrementing constructor was called.
		~Incrementer() { this->counter--; }

		// Forbid copy (and move).
		Incrementer(Incrementer const &) = delete;
		Incrementer &operator=(Incrementer const &) = delete;
	};
}

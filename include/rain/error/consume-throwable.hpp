// Provide a wrapper which noexcepts any generic callable by consuming its
// exceptions.
#pragma once

#include "../functional.hpp"
#include "../string/string.hpp"

#include <iostream>

// Retrieves caller site for debugging.
#define RAIN_ERROR_LOCATION __FILE__ ":" STRINGIFY(__LINE__)

namespace Rain::Error {
	// Wraps any generic callable in try/catch, consuming all exceptions to stderr
	// (optional).
	//
	// The return type of the callable must be default-constructable in case of
	// throw. Namely, char const * may error as its default constructor does not
	// necessarily point to a valid location.
	//
	// consumeThrowable must be called with Args... matching the parameters to the
	// callable. It does not support default arguments in the callable, as Args...
	// does not accomodate that signature.
	//
	// Crashes may still be caused via signal crashes (dividing by 0, etc.).
	template <typename Callable>
	auto consumeThrowable(
		// Perfect forwarding rvalue-reference to allow for inline construction and
		// later move-capture.
		Callable &&callable,
		// RAIN_ERROR_LOCATION will identify caller site on-throw. Otherwise, an
		// empty location will silently cons.ume.
		std::string const &location = "") {
		// Perfect-forward the callable in. Preserves mutability, copies lambdas &
		// function pointers, but moves std::functions if requested.
		return [capturedCallable = std::forward<Callable>(callable),
						location](auto &&...args) mutable {
			try {
				return capturedCallable(std::forward<decltype(args)>(args)...);
			} catch (std::exception const &exception) {
				// Output exception if possible.
				if (!location.empty()) {
					std::cerr << exception.what();
				}
			} catch (...) {
				// Consume generic exception, do nothing.
			}

			if (!location.empty()) {
				std::cerr << "Consumed exception at " << location << "." << std::endl;
			}

			// Gets the return type of the callable at compile-time, and invokes its
			// default constructor at runtime.
			//
			// Allows void return type as well.
			return decltype(capturedCallable(
				std::forward<decltype(args)>(args)...))();
		};
	}
}

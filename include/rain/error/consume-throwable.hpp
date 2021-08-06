// Provide a wrapper which noexcepts any generic callable by consuming its
// exceptions.
#pragma once

#include "../string/string.hpp"

#include <functional>
#include <iostream>

// Retrieves caller site for debugging.
#define RAIN_ERROR_LOCATION __FILE__ ":" STRINGIFY(__LINE__)

namespace Rain::Error {
	// Wraps any generic callable in try/catch, consuming all exceptions to stderr
	// (optional).
	//
	// The return type of the callable must be default-constructable in case of
	// throw. Namely char const * may error as its default constructor does not
	// necessarily point to a valid location.
	//
	// consumeThrowable must be called with Args... matching the parameters to the
	// callable. It does not support default arguments in the callable, as Args...
	// does not accomodate that signature.
	//
	// Crashes may still be caused via signal crashes (dividing by 0, etc.).
	template <typename... Args, typename Callable>
	auto consumeThrowable(
		Callable const &callable,
		// RAIN_ERROR_LOCATION will identify caller site on-throw. Otherwise, an
		// empty location will silently consume.
		std::string const &location = "") {
		return [callable, location](Args... args) {
			try {
				return callable(std::forward<Args>(args)...);
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
			return typename decltype(std::function{callable})::result_type();
		};
	}
}

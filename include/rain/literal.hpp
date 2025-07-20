// User-defined literals. Literals are meant to be resolved through the
// namespace scope: using namespace Rain::Literal;.
#pragma once

// Provides std::literals.
#include <chrono>

#include <regex>

namespace Rain {
	// Inline namespaces are implicitly accessible by the parent namespace.
	inline namespace Literal {
		// Inject std literals into the Rain namespace. Injecting the entire
		// namespace via `using namespace std::literals` may cause compilation
		// issues with default arguments using those literals.
		using std::string_literals::operator""s;
		using std::chrono_literals::operator""h;
		using std::chrono_literals::operator""s;
		using std::chrono_literals::operator""ms;

		// User-defined literals.
		inline constexpr std::size_t operator""_zu(unsigned long long value) {
			return static_cast<std::size_t>(value);
		}
		inline std::regex operator""_re(char const *value, std::size_t) {
			return std::regex(value);
		}
		inline constexpr short operator""_ss(unsigned long long value) {
			return static_cast<short>(value);
		}
	}
}

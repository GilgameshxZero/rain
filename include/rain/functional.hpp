// Additional functional utilities.
#pragma once

#include <functional>
#include <type_traits>

// For an overloaded function f, this wraps it in an rvalue-reference lambda so
// that it may be resolved via perfect forwarding.
#define RAIN_FUNCTIONAL_RESOLVE_OVERLOAD(f)          \
	[](auto &&...args) -> decltype(auto) {             \
		return f(std::forward<decltype(args)>(args)...); \
	}

namespace Rain::Functional {
	// Simple function to combine two hashes, based on
	// <https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x>.
	template <class T>
	inline void combineHash(std::size_t &seed, T const &value) {
		seed ^= std::hash<>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	// SFINAE for const-iterable types (containers). Assumes sizeof(char) and
	// sizeof(int) are not equal.
	template <typename Type>
	struct isConstIterable {
		template <typename TypeInner>
		static char test(typename TypeInner::const_iterator *);
		template <typename TypeInner>
		static int test(...);

		public:
		enum { value = sizeof(test<Type>(0)) == sizeof(char) };
	};

	// Custom hash functor for const-iterable containers. Cannot be placed in std
	// namespace as it is UB for system types (not UB for user-defined types).
	template <
		typename Container,
		typename std::enable_if<isConstIterable<Container>::value>::type * =
			nullptr>
	struct ContainerHash {
		std::size_t operator()(Container const &value) const {
			std::size_t result{0};
			for (auto const &i : value) {
				combineHash(result, i);
			}
			return result;
		}
	};

	// Custom hash functor for std::pair (TODO: implement tuple functor).
	template <typename TypeFirst, typename TypeSecond>
	struct PairHash {
		std::size_t operator()(
			std::pair<TypeFirst, TypeSecond> const &value) const {
			std::size_t result{std::hash<>{}(value.first)};
			combineHash(result, value.second);
			return result;
		}
	};
}

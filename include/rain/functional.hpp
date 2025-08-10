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
	// SFINAE for const-iterable types (containers). Assumes sizeof(char) and
	// sizeof(int) are not equal.
	template <typename Type>
	struct isConstIterable {
		template <typename TypeInner>
		static char evaluate(typename TypeInner::const_iterator *);
		template <typename TypeInner>
		static int evaluate(...);

		public:
		enum { value = sizeof(evaluate<Type>(0)) == sizeof(char) };
	};

	// Similar to `std::is_base_of`, but for template base types.
	namespace {
		template <template <typename...> class Type, typename... TypeTemplate>
		std::true_type isBaseOfTemplateImpl(Type<TypeTemplate...> const *);
		template <template <typename...> class Type>
		std::false_type isBaseOfTemplateImpl(...);
	}
	template <template <typename...> class TypeBase, typename TypeDerived>
	using isBaseOfTemplate =
		decltype(isBaseOfTemplateImpl<TypeBase>(std::declval<TypeDerived *>()));

	template <typename Type, typename = std::void_t<>>
	struct isStdHashable : std::false_type {};
	template <typename Type>
	struct isStdHashable<
		Type,
		std::void_t<decltype(std::declval<std::hash<Type>>()(
			std::declval<Type>()))>> : std::true_type {};
}

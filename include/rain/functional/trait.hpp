#pragma once

#include "../platform.hpp"

#include <functional>
#include <type_traits>

namespace Rain::Functional {
	// Similar to `integral_constant`, upgrades a value into a
	// type, but with automatic type deduction on the value.
	template<auto VALUE>
	class TypeUpgrade {
		public:
		static inline auto constexpr UNDERLYING{VALUE};
	};
	// Downgrades a non-type, non-constant template parameter
	// into a type template parameter. Useful to wrap more
	// complex template template types before passing to
	// generic templates accepting `typename...`.
	template<template<typename...> typename Type>
	class TypeDowngrade {
		public:
		template<typename... Args>
		using Underlying = Type<Args...>;
	};

	template<typename... Types>
	class TypeTrait;

	// Type traits for parameter packs where nothing is known.
	// Unpacks variadic templates if possible.
	template<typename... Types>
	class TypeTraitInterface {
		public:
		using HasTypeFirst = std::false_type;
	};
	template<typename First, typename... Types>
	class TypeTraitInterface<First, Types...> {
		public:
		using HasTypeFirst = std::true_type;
		using TypeFirst = First;

		// Get second type via Remaining::First, third type
		// via Remaining::Remaining::First, and so on.
		using TypeTraitRemaining = TypeTrait<Types...>;
	};

	// Type traits for a single type whose upgrade/downgrade
	// status is unknown.
	template<typename>
	class TypeTraitInterfaceFirstInterface {
		public:
		using IsTypeUpgrade = std::false_type;
		using IsTypeSidegrade = std::true_type;
		using IsTypeDowngrade = std::false_type;
	};
	template<auto VALUE>
	class TypeTraitInterfaceFirstInterface<
		TypeUpgrade<VALUE>> {
		public:
		using IsTypeUpgrade = std::true_type;
		using IsTypeSidegrade = std::false_type;
		using IsTypeDowngrade = std::false_type;
	};
	template<template<typename...> typename Type>
	class TypeTraitInterfaceFirstInterface<
		TypeDowngrade<Type>> {
		public:
		using IsTypeUpgrade = std::false_type;
		using IsTypeSidegrade = std::false_type;
		using IsTypeDowngrade = std::true_type;
	};

	// Type traits for a single type which is an upgrade.
	template<typename, typename = void>
	class TypeTraitInterfaceFirstInterfaceUpgrade {};
	template<typename Type>
	class TypeTraitInterfaceFirstInterfaceUpgrade<
		Type,
		typename std::enable_if<
			TypeTraitInterfaceFirstInterface<
				Type>::IsTypeUpgrade::value>::type> {
		public:
		// Usage of these scoped templates as a template
		// template parameter may require the use of the
		// `template` keyword if the outer template is
		// dependent (e.g., `TraitAuto<SIZE>::template
		// isEqualTo`).
		template<auto RIGHT>
		using IsEqualTo = std::
			integral_constant<bool, Type::UNDERLYING == RIGHT>;
		template<auto RIGHT>
		using IsLessThan = std::
			integral_constant<bool, (Type::UNDERLYING < RIGHT)>;
		template<auto RIGHT>
		using IsGreaterThan = std::
			integral_constant<bool, (Type::UNDERLYING > RIGHT)>;
	};

	// Individual overrideable type traits for Sidegrade
	// type arithmetic traits.
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsIntegral {
		public:
		static inline bool constexpr value{
			std::is_integral<Type>::value};
	};
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsFloatingPoint {
		public:
		static inline bool constexpr value{
			std::is_floating_point<Type>::value};
	};
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic {
		public:
		static inline bool constexpr value{
			TypeTraitInterfaceFirstInterfaceSidegradeIsIntegral<
				Type>::value ||
			TypeTraitInterfaceFirstInterfaceSidegradeIsFloatingPoint<
				Type>::value};
	};
	template<typename, typename = void>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsSignedInterface;
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsSignedInterface<
			Type,
			typename std::enable_if<
				!TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic<
					Type>::value>::type> {
		public:
		static inline bool constexpr value{false};
	};
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsSignedInterface<
			Type,
			typename std::enable_if<
				TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic<
					Type>::value>::type> {
		public:
		static inline bool constexpr value{Type(-1) < Type(0)};
	};
	template<typename Type>
	class TypeTraitInterfaceFirstInterfaceSidegradeIsSigned :
		public TypeTraitInterfaceFirstInterfaceSidegradeIsSignedInterface<
			Type> {};
	template<typename, typename = void>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsUnsignedInterface;
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsUnsignedInterface<
			Type,
			typename std::enable_if<
				!TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic<
					Type>::value>::type> {
		public:
		static inline bool constexpr value{false};
	};
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsUnsignedInterface<
			Type,
			typename std::enable_if<
				TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic<
					Type>::value>::type> {
		public:
		static inline bool constexpr value{Type(0) < Type(-1)};
	};
	template<typename Type>
	class
		TypeTraitInterfaceFirstInterfaceSidegradeIsUnsigned :
		public TypeTraitInterfaceFirstInterfaceSidegradeIsUnsignedInterface<
			Type> {};

	// Type traits for a single type/sidegrade.
	template<typename, typename = void>
	class TypeTraitInterfaceFirstInterfaceSidegrade {};
	template<typename Type>
	class TypeTraitInterfaceFirstInterfaceSidegrade<
		Type,
		typename std::enable_if<
			TypeTraitInterfaceFirstInterface<
				Type>::IsTypeSidegrade::value>::type> {
		public:
		template<typename>
		static std::false_type isBaseOf(...);
		template<typename TypeBase>
		static std::true_type isBaseOf(TypeBase const *);
		template<
			typename TypeDerived,
			std::enable_if<TypeTraitInterfaceFirstInterface<
				TypeDerived>::IsTypeSidegrade::value>::type * =
				nullptr>
		using IsBaseOf =
			decltype(isBaseOf<typename std::decay<Type>::type>(
				std::declval<
					typename std::decay<TypeDerived>::type *>()));

		template<typename>
		static std::false_type isStdHashable(...);
		template<typename T>
		static std::true_type isStdHashable(
			decltype(std::hash<T>()) *);
		using IsStdHashable = decltype(isStdHashable<
			typename std::decay<Type>::type>(nullptr));

		template<typename>
		static std::false_type isConstIterable(...);
		template<typename T>
		static std::true_type isConstIterable(
			typename T::const_iterator *);
		using IsConstIterable = decltype(isConstIterable<
			typename std::decay<Type>::type>(nullptr));

		template<typename, typename...>
		static std::false_type isCallableWith(...);
		template<typename T, typename... Args>
		static std::true_type isCallableWith(
			decltype(std::declval<T>()(
				std::declval<Args...>())) *);
		template<typename... Args>
		using IsCallableWith = decltype(isCallableWith<
			typename std::decay<Type>::type,
			Args...>(nullptr));

		// Classical arithmetic type traits.
		using IsIntegral =
			TypeTraitInterfaceFirstInterfaceSidegradeIsIntegral<
				typename std::decay<Type>::type>;
		using IsFloatingPoint =
			TypeTraitInterfaceFirstInterfaceSidegradeIsFloatingPoint<
				typename std::decay<Type>::type>;
		using IsArithmetic =
			TypeTraitInterfaceFirstInterfaceSidegradeIsArithmetic<
				typename std::decay<Type>::type>;
		using IsSigned =
			TypeTraitInterfaceFirstInterfaceSidegradeIsSigned<
				typename std::decay<Type>::type>;
		using IsUnsigned =
			TypeTraitInterfaceFirstInterfaceSidegradeIsUnsigned<
				typename std::decay<Type>::type>;
	};

	// Type traits for a single type which is a downgrade.
	template<typename, typename = void>
	class TypeTraitInterfaceFirstInterfaceDowngrade {};
	template<typename Type>
	class TypeTraitInterfaceFirstInterfaceDowngrade<
		Type,
		typename std::enable_if<
			TypeTraitInterfaceFirstInterface<
				Type>::IsTypeDowngrade::value>::type> {
		public:
		// Template specification detection is actually very
		// difficult in this context, largely due to
		// TypeUnderlying being a alias template dependent type,
		// and type deduction failing in general for alias
		// template types. Thus, the standard method of deducing
		// types args (via class template) for TypeSpecific will
		// fail to compile.
		//
		// We bypass this with a deductor based on function
		// arguments. Furthermore, we must define type alias
		// TypeUnderlying or the compiler will fail to see the
		// dependent type as a template type.
		//
		// Some links follow.
		//
		// Before C++20, alias templates are never deduced:
		// <https://en.cppreference.com/cpp/language/template_argument_deduction>.
		// Discussion:
		// <https://stackoverflow.com/questions/41008092/class-template-argument-deduction-not-working-with-alias-template>.
		// Simply trying to deduce TypeUnderlying<Args...> will
		// fail because the dependent type TypeUnderlying names
		// a non-deducible context:
		// <https://stackoverflow.com/questions/25245453/what-is-a-non-deduced-context>.
		//
		// Using a reference instead of a pointer should prevent
		// decays into base types. To detect base types, use
		// specific types with the Sidegrade version of
		// IsBaseOf.
		template<typename... Args>
		using TypeUnderlying =
			typename Type::template Underlying<Args...>;

		// Detects if Type is the template of a base of
		// TypeSpecific.
		template<template<typename...> typename>
		static std::false_type isTemplateBaseOf(...);
		template<
			template<typename...> typename TypeBase,
			typename... Args>
		static std::true_type isTemplateBaseOf(
			TypeBase<Args...> const *);
		// MSVC has a bug where template type deduction here
		// doesn't properly take into account the variadic
		// Args... So we manually overload for up to 2
		// arguments.
		template<
			template<typename...> typename TypeBase,
			typename Arg1,
			typename Arg2>
		static std::true_type isTemplateBaseOf(
			TypeBase<Arg1, Arg2> const *);

		template<typename TypeSpecific>
		using IsTemplateBaseOf =
			decltype(isTemplateBaseOf<TypeUnderlying>(
				std::declval<
					typename std::decay<TypeSpecific>::type *>()));

		// To determine if TypeSpecific is a specification of
		// the Type template, but not a derived class of it,
		// Type must be at least a TemplateBase (template and/or
		// base) of TypeSpecific. Then, we can take the "middle"
		// type, which is a specification of Type. If the middle
		// type is equal to TypeSpecific, then we are not a
		// base, just a template.
		template<typename, typename, typename = void>
		class IsTemplateOfInterface : public std::false_type {};
		template<typename TypeSpecific, typename TypeMiddle>
		class IsTemplateOfInterface<
			TypeSpecific,
			TypeMiddle,
			typename std::enable_if<std::
					is_same<TypeSpecific, TypeMiddle>::value>::type> :
			public std::true_type {};

		template<typename, template<typename...> typename>
		static std::false_type isTemplateOf(...);
		template<
			typename TypeSpecific,
			template<typename...> typename TypeBase,
			typename... Args>
		static IsTemplateOfInterface<
			TypeSpecific,
			TypeBase<Args...>>
			isTemplateOf(TypeBase<Args...> const *);
		// MSVC bug here again. TODO: file it.
		template<
			typename TypeSpecific,
			template<typename...> typename TypeBase,
			typename Arg1,
			typename Arg2>
		static IsTemplateOfInterface<
			TypeSpecific,
			TypeBase<Arg1, Arg2>>
			isTemplateOf(TypeBase<Arg1, Arg2> const *);

		template<typename TypeSpecific>
		using IsTemplateOf = decltype(isTemplateOf<
			typename std::decay<TypeSpecific>::type,
			TypeUnderlying>(std::declval<
			typename std::decay<TypeSpecific>::type *>()));

		// Detects if Type<Args...> is a defined type.
		template<template<typename...> typename, typename...>
		static std::false_type isSpecifiedBy(...);
		template<
			template<typename...> typename TypeTemplate,
			typename... Args>
		static std::true_type isSpecifiedBy(
			decltype(TypeTemplate<Args...>()) *);
		template<typename... Args>
		using IsSpecifiedBy = decltype(isSpecifiedBy<
			TypeUnderlying,
			typename std::decay<Args>::type...>(nullptr));
	};

	// Type traits for a parameter pack where at least the
	// first parameter exists.
	template<typename...>
	class TypeTraitInterfaceFirst {};
	template<typename First, typename... Types>
	class TypeTraitInterfaceFirst<First, Types...> :
		public TypeTraitInterfaceFirstInterface<First>,
		public TypeTraitInterfaceFirstInterfaceUpgrade<First>,
		public TypeTraitInterfaceFirstInterfaceSidegrade<First>,
		public TypeTraitInterfaceFirstInterfaceDowngrade<
			First> {};

	// Type traits inherits from interfaces, which deduces
	// properties about the parameter pack, step by step.
	template<typename... Types>
	class TypeTrait :
		public TypeTraitInterface<Types...>,
		public TypeTraitInterfaceFirst<Types...> {};
}

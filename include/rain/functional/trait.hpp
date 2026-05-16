#pragma once

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

	// Type traits for a single type which is a downgrade.
	template<typename, typename = void>
	class TypeTraitInterfaceFirstInterfaceSidegrade {};
	template<typename Type>
	class TypeTraitInterfaceFirstInterfaceSidegrade<
		Type,
		typename std::enable_if<
			TypeTraitInterfaceFirstInterface<
				Type>::IsTypeSidegrade::value>::type> {
		private:
		template<typename>
		static std::false_type isBaseOf(...);
		template<typename TypeBase>
		static std::true_type isBaseOf(TypeBase const *);

		public:
		template<
			typename TypeDerived,
			std::enable_if<TypeTraitInterfaceFirstInterface<
				TypeDerived>::IsTypeSidegrade::value>::type * =
				nullptr>
		using IsBaseOf = decltype(isBaseOf<Type>(
			std::declval<TypeDerived *>()));

		private:
		template<typename>
		static std::false_type isStdHashable(...);
		template<typename T>
		static std::true_type isStdHashable(
			decltype(std::hash<T>()) *);

		public:
		using IsStdHashable =
			decltype(isStdHashable<Type>(nullptr));

		private:
		template<typename>
		static std::false_type isConstIterable(...);
		template<typename T>
		static std::true_type isConstIterable(
			typename T::const_iterator *);

		public:
		using IsConstIterable =
			decltype(isConstIterable<Type>(nullptr));

		private:
		template<typename, typename...>
		static std::false_type isCallableWith(...);
		template<typename T, typename... Args>
		static std::true_type isCallableWith(
			decltype(std::declval<T>()(
				std::declval<Args...>())) *);

		public:
		template<typename... Args>
		using IsCallableWith =
			decltype(isCallableWith<Type, Args...>(nullptr));

		// Classical type traits.
		private:
		struct IsIntegralDefault {
			static inline bool constexpr value{
				std::is_integral<Type>::value};
		};
		struct IsIntegralCustom {
			static inline bool constexpr value{
				Type::Trait::IS_INTEGRAL};
		};
		template<typename>
		static IsIntegralDefault isIntegral(...);
		template<typename T>
		static IsIntegralCustom isIntegral(
			decltype(T::Trait::IS_INTEGRAL) *);

		public:
		using IsIntegral = decltype(isIntegral<Type>(nullptr));

		private:
		struct IsFloatingPointDefault {
			static inline bool constexpr value{
				std::is_floating_point<Type>::value};
		};
		struct IsFloatingPointCustom {
			static inline bool constexpr value{
				Type::Trait::IS_FLOATING_POINT};
		};
		template<typename>
		static IsFloatingPointDefault isFloatingPoint(...);
		template<typename T>
		static IsFloatingPointCustom isFloatingPoint(
			decltype(T::Trait::IS_FLOATING_POINT) *);

		public:
		using IsFloatingPoint =
			decltype(isFloatingPoint<Type>(nullptr));

		private:
		struct IsArithmeticDefault {
			static inline bool constexpr value{
				IsIntegral::value || IsFloatingPoint::value};
		};
		struct IsArithmeticCustom {
			static inline bool constexpr value{
				Type::Trait::IS_ARITHMETIC};
		};
		template<typename>
		static IsArithmeticDefault isArithmetic(...);
		template<typename T>
		static IsArithmeticCustom isArithmetic(
			decltype(T::Trait::IS_ARITHMETIC) *);

		public:
		using IsArithmetic =
			decltype(isArithmetic<Type>(nullptr));

		private:
		struct IsSignedDefaultInnerTrue {
			static inline bool constexpr value{
				Type(-1) < Type(0)};
		};
		struct IsSignedDefaultInnerFalse {
			static inline bool constexpr value{false};
		};
		template<typename>
		static IsSignedDefaultInnerFalse isSignedDefaultInner(
			...);
		template<typename T>
		static IsSignedDefaultInnerTrue isSignedDefaultInner(
			std::enable_if<T::value>::type *);
		using IsSignedDefault =
			decltype(isSignedDefaultInner<IsArithmetic>(nullptr));

		struct IsSignedCustom {
			static inline bool constexpr value{
				Type::Trait::IS_SIGNED};
		};
		template<typename>
		static IsSignedDefault isSigned(...);
		template<typename T>
		static IsSignedCustom isSigned(
			decltype(T::Trait::IS_SIGNED) *);

		public:
		using IsSigned = decltype(isSigned<Type>(nullptr));

		private:
		struct IsUnsignedDefaultInner {
			static inline bool constexpr value{
				Type(0) < Type(-1)};
		};
		template<typename>
		static std::false_type isUnsignedDefaultInner(...);
		template<typename T>
		static IsUnsignedDefaultInner isUnsignedDefaultInner(
			std::enable_if<T::value>::type *);
		using IsUnsignedDefault =
			decltype(isUnsignedDefaultInner<IsArithmetic>(
				nullptr));

		struct IsUnsignedCustom {
			static inline bool constexpr value{
				Type::Trait::IS_UNSIGNED};
		};
		template<typename>
		static IsUnsignedDefault isUnsigned(...);
		template<typename T>
		static IsUnsignedCustom isUnsigned(
			decltype(T::Trait::IS_UNSIGNED) *);

		public:
		using IsUnsigned = decltype(isUnsigned<Type>(nullptr));
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
		private:
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

		public:
		template<
			typename TypeSpecific,
			std::enable_if<TypeTraitInterfaceFirstInterface<
				TypeSpecific>::IsTypeSidegrade::value>::type * =
				nullptr>
		using IsTemplateBaseOf =
			decltype(isTemplateBaseOf<TypeUnderlying>(
				std::declval<TypeSpecific *>()));

		// To determine if TypeSpecific is a specification of
		// the Type template, but not a derived class of it,
		// Type must be at least a TemplateBase (template and/or
		// base) of TypeSpecific. Then, we can take the "middle"
		// type, which is a specification of Type. If the middle
		// type is equal to TypeSpecific, then we are not a
		// base, just a template.
		private:
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

		public:
		template<
			typename TypeSpecific,
			std::enable_if<TypeTraitInterfaceFirstInterface<
				TypeSpecific>::IsTypeSidegrade::value>::type * =
				nullptr>
		using IsTemplateOf =
			decltype(isTemplateOf<TypeSpecific, TypeUnderlying>(
				std::declval<TypeSpecific *>()));
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

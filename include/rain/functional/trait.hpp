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

		template<template<typename...> typename TypeBase>
		static std::false_type isTemplateOf(...);
		template<
			template<typename...> typename TypeBase,
			typename... Args>
		static std::true_type isTemplateOf(
			TypeBase<Args...> const &);

		public:
		template<
			typename TypeSpecific,
			std::enable_if<TypeTraitInterfaceFirstInterface<
				TypeSpecific>::IsTypeSidegrade::value>::type * =
				nullptr>
		using IsTemplateOf =
			decltype(isTemplateOf<TypeUnderlying>(
				std::declval<TypeSpecific &>()));
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

	// True/false types for ease-of-use later. We prefer
	// these of the std ones because of naming conventions
	// (`VALUE` instead of `value`).
	//
	// TODO: remove.
	struct TraitTrue {
		static inline bool constexpr VALUE{true};
	};
	struct TraitFalse {
		static inline bool constexpr VALUE{false};
	};

	// Specialize by overriding select variables in the
	// Type::Trait type.
	//
	// TODO: replace with TypeTrait.
	template<typename Type>
	class TraitType {
		public:
		// Classic type traits.
		struct IsIntegralDefault {
			static inline bool constexpr VALUE{
				std::is_integral<Type>::value};
		};
		struct IsIntegralCustom {
			static inline bool constexpr VALUE{
				Type::Trait::IS_INTEGRAL};
		};
		template<typename>
		static IsIntegralDefault isIntegral(...);
		template<typename T>
		static IsIntegralCustom isIntegral(
			decltype(T::Trait::IS_INTEGRAL) *);
		using IsIntegral = decltype(isIntegral<Type>(nullptr));

		struct IsFloatingPointDefault {
			static inline bool constexpr VALUE{
				std::is_floating_point<Type>::value};
		};
		struct IsFloatingPointCustom {
			static inline bool constexpr VALUE{
				Type::Trait::IS_FLOATING_POINT};
		};
		template<typename>
		static IsFloatingPointDefault isFloatingPoint(...);
		template<typename T>
		static IsFloatingPointCustom isFloatingPoint(
			decltype(T::Trait::IS_FLOATING_POINT) *);
		using IsFloatingPoint =
			decltype(isFloatingPoint<Type>(nullptr));

		struct IsArithmeticDefault {
			static inline bool constexpr VALUE{
				IsIntegral::VALUE || IsFloatingPoint::VALUE};
		};
		struct IsArithmeticCustom {
			static inline bool constexpr VALUE{
				Type::Trait::IS_ARITHMETIC};
		};
		template<typename>
		static IsArithmeticDefault isArithmetic(...);
		template<typename T>
		static IsArithmeticCustom isArithmetic(
			decltype(T::Trait::IS_ARITHMETIC) *);
		using IsArithmetic =
			decltype(isArithmetic<Type>(nullptr));

		struct IsSignedDefaultInnerTrue {
			static inline bool constexpr VALUE{
				Type(-1) < Type(0)};
		};
		struct IsSignedDefaultInnerFalse {
			static inline bool constexpr VALUE{false};
		};
		template<typename>
		static IsSignedDefaultInnerFalse isSignedDefaultInner(
			...);
		template<typename T>
		static IsSignedDefaultInnerTrue isSignedDefaultInner(
			std::enable_if<T::VALUE>::type *);
		using IsSignedDefault =
			decltype(isSignedDefaultInner<IsArithmetic>(nullptr));

		struct IsSignedCustom {
			static inline bool constexpr VALUE{
				Type::Trait::IS_SIGNED};
		};
		template<typename>
		static IsSignedDefault isSigned(...);
		template<typename T>
		static IsSignedCustom isSigned(
			decltype(T::Trait::IS_SIGNED) *);
		using IsSigned = decltype(isSigned<Type>(nullptr));

		struct IsUnsignedDefaultInner {
			static inline bool constexpr VALUE{
				Type(0) < Type(-1)};
		};
		template<typename>
		static TraitFalse isUnsignedDefaultInner(...);
		template<typename T>
		static IsUnsignedDefaultInner isUnsignedDefaultInner(
			std::enable_if<T::VALUE>::type *);
		using IsUnsignedDefault =
			decltype(isUnsignedDefaultInner<IsArithmetic>(
				nullptr));

		struct IsUnsignedCustom {
			static inline bool constexpr VALUE{
				Type::Trait::IS_UNSIGNED};
		};
		template<typename>
		static IsUnsignedDefault isUnsigned(...);
		template<typename T>
		static IsUnsignedCustom isUnsigned(
			decltype(T::Trait::IS_UNSIGNED) *);
		using IsUnsigned = decltype(isUnsigned<Type>(nullptr));

		// Custom type traits.
		template<typename>
		static TraitFalse isStdHashable(...);
		template<typename T>
		static TraitTrue isStdHashable(
			decltype(std::hash<T>()) *);
		using IsStdHashable =
			decltype(isStdHashable<Type>(nullptr));

		template<typename>
		static TraitFalse isConstIterable(...);
		template<typename T>
		static TraitTrue isConstIterable(
			typename T::const_iterator *);
		using IsConstIterable =
			decltype(isConstIterable<Type>(nullptr));

		template<typename, typename...>
		static TraitFalse isCallableWith(...);
		template<typename T, typename... Args>
		static TraitTrue isCallableWith(
			decltype(std::declval<T>()(
				std::declval<Args...>())) *);
		template<typename... Args>
		using IsCallableWith =
			decltype(isCallableWith<Type, Args...>(nullptr));
	};

	// Type traits for template types.
	// TODO: Replace with TypeTrait.
	template<template<typename...> typename Type>
	class TraitTypeTemplate {
		public:
		template<template<typename...> typename>
		static TraitFalse isBaseOfTemplate(...);
		template<
			template<typename...> typename TypeInner,
			typename... TypeTemplate>
		static TraitTrue isBaseOfTemplate(
			TypeInner<TypeTemplate...> const *);
		template<typename TypeDerived>
		using IsBaseOfTemplate =
			decltype(isBaseOfTemplate<Type>(
				std::declval<TypeDerived *>()));
	};
	template<template<auto...> typename Type>
	class TraitTypeTemplateAuto {
		public:
		template<template<auto...> typename>
		static TraitFalse isBaseOfTemplate(...);
		template<
			template<auto...> typename TypeInner,
			auto... TypeTemplate>
		static TraitTrue isBaseOfTemplate(
			TypeInner<TypeTemplate...> const *);
		template<typename TypeDerived>
		using IsBaseOfTemplate =
			decltype(isBaseOfTemplate<Type>(
				std::declval<TypeDerived *>()));
	};
	template<template<typename, auto...> typename Type>
	class TraitTypeTemplateTypeAuto {
		public:
		template<template<typename, auto...> typename>
		static TraitFalse isBaseOfTemplate(...);
		template<
			template<typename, auto...> typename TypeInner,
			typename TypeTemplateFirst,
			auto... TypeTemplate>
		static TraitTrue isBaseOfTemplate(
			TypeInner<TypeTemplateFirst, TypeTemplate...> const
				*);
		template<typename TypeDerived>
		using IsBaseOfTemplate =
			decltype(isBaseOfTemplate<Type>(
				std::declval<TypeDerived *>()));
	};
	template<
		template<typename, typename, auto...> typename Type>
	class TraitTypeTemplateTypeTypeAuto {
		public:
		template<template<typename, typename, auto...> typename>
		static TraitFalse isBaseOfTemplate(...);
		template<
			template<
				typename,
				typename,
				auto...> typename TypeInner,
			typename TypeTemplateFirst,
			typename TypeTemplateSecond,
			auto... TypeTemplate>
		static TraitTrue isBaseOfTemplate(
			TypeInner<
				TypeTemplateFirst,
				TypeTemplateSecond,
				TypeTemplate...> const *);
		template<typename TypeDerived>
		using IsBaseOfTemplate =
			decltype(isBaseOfTemplate<Type>(
				std::declval<TypeDerived *>()));
	};
}

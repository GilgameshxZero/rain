#pragma once

#include <functional>
#include <type_traits>

namespace Rain::Functional {
	// True/false types for ease-of-use later. We prefer these
	// of the std ones because of naming conventions.
	struct TraitTrue {
		static inline bool constexpr VALUE{true};
	};
	struct TraitFalse {
		static inline bool constexpr VALUE{false};
	};

	// "Traits" for an std::size_t type template argument.
	template<std::size_t LEFT>
	class TraitSizeT {
		public:
		// Usage of these scoped templates as a template
		// template parameter may require the use of the
		// `template` keyword if the outer template is dependent
		// (e.g., `Property<SIZE>::template isEqualTo`).
		template<std::size_t RIGHT>
		struct IsEqualTo {
			static inline bool constexpr VALUE{LEFT == RIGHT};
		};
		template<std::size_t RIGHT>
		struct IsLessThan {
			static inline bool constexpr VALUE{LEFT < RIGHT};
		};
		template<std::size_t RIGHT>
		struct IsGreaterThan {
			static inline bool constexpr VALUE{LEFT < RIGHT};
		};
	};

	// Specialize by overriding select variables in the
	// Type::Trait type.
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
	};

	// Type traits for template types.
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

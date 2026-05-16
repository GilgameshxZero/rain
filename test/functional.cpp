#include <rain.hpp>

using namespace Rain;
using namespace Functional;
using namespace Math;

using namespace std;

class Type1 {
	public:
	struct Trait {};
};

class Type2 {};

namespace Rain::Functional {
	template<>
	class TypeTraitInterfaceFirstInterfaceSidegradeIsIntegral<
		Type2> {
		public:
		static inline bool constexpr value{true};
	};
	template<>
	class TypeTraitInterfaceFirstInterfaceSidegradeIsSigned<
		Type2> {
		public:
		static inline bool constexpr value{false};
	};
}

template<typename, typename>
class Type3 {};

class Type4 :
	public Type3<TypeUpgrade<4>, TypeUpgrade<true>> {};

template<typename>
class A {};

class B {};

template<typename>
class C {};

// E base class which selectively defines member variable
// via SFINAE.
template<typename, typename = void>
class EBase {};

template<typename Type>
class EBase<
	Type,
	typename enable_if<Functional::TypeTrait<
		Type>::IsTypeUpgrade::value>::type> {
	public:
	static inline auto constexpr SECOND{Type::UNDERLYING};
};

// Accept all kinds of types and templates.
template<typename... T>
class E :
	public EBase<typename Functional::TypeTrait<
		T...>::TypeTraitRemaining::TypeFirst> {
	public:
	static inline auto constexpr FIRST{
		Functional::TypeTrait<T...>::TypeFirst::UNDERLYING};
};

int main() {
	// Must include <array>.
	static_assert(TypeTrait<long long>::IsStdHashable::value);
	static_assert(
		!TypeTrait<std::array<int, 8>>::IsStdHashable::value);
	static_assert(
		!(TypeTrait<TypeDowngrade<std::pair>>::IsTemplateOf<
			std::array<int, 8>>::value));
	static_assert(
		TypeTrait<TypeDowngrade<std::pair>>::IsTemplateOf<
			std::pair<int, bool>>::value);
	static_assert(
		TypeTrait<std::array<int, 8>>::IsConstIterable::value);

	// Cannot use std::is_base_of with template base types.
	static_assert(
		TypeTrait<TypeDowngrade<std::pair>>::IsTemplateOf<
			std::pair<int, char>>::value);
	static_assert(!TypeTrait<TypeDowngrade<std::vector>>::
			IsTemplateBaseOf<std::pair<int, char>>::value);
	static_assert(
		TypeTrait<TypeDowngrade<std::vector>>::IsTemplateOf<
			std::vector<int>>::value);

	static_assert(
		!TypeTrait<TypeDowngrade<Type3>>::IsTemplateBaseOf<
			std::vector<int>>::value);
	static_assert(TypeTrait<
		TypeDowngrade<Type3>>::IsTemplateBaseOf<Type4>::value);
	static_assert(!TypeTrait<
		TypeDowngrade<Type3>>::IsTemplateOf<Type4>::value);

	static_assert(!TypeTrait<std::size_t>::IsSigned::value);

	static_assert(!TypeTrait<Type1>::IsIntegral::value);
	static_assert(!TypeTrait<Type1>::IsFloatingPoint::value);
	static_assert(!TypeTrait<Type1>::IsArithmetic::value);
	static_assert(!TypeTrait<Type1>::IsSigned::value);

	static_assert(TypeTrait<Type2>::IsIntegral::value);
	static_assert(!TypeTrait<Type2>::IsFloatingPoint::value);
	static_assert(TypeTrait<Type2>::IsArithmetic::value);
	static_assert(!TypeTrait<Type2>::IsSigned::value);

	static_assert(
		TypeTrait<BigIntegerFlexUnsigned>::IsIntegral::value);
	static_assert(
		!TypeTrait<BigIntegerFlexUnsigned>::IsSigned::value);
	static_assert(
		TypeTrait<BigIntegerFlexUnsigned>::IsUnsigned::value);
	static_assert(
		TypeTrait<BigIntegerSigned<7>>::IsIntegral::value);
	static_assert(
		TypeTrait<BigIntegerSigned<7>>::IsSigned::value);
	static_assert(
		!TypeTrait<BigIntegerSigned<7>>::IsUnsigned::value);

	{
		E<Functional::TypeUpgrade<77>,
			Functional::TypeDowngrade<A>,
			B,
			Functional::TypeDowngrade<C>>
			e;
		static_assert(e.FIRST == 77);
		// static_assert(e.SECOND == 77);
	}

	{
		E<Functional::TypeUpgrade<77>,
			Functional::TypeUpgrade<34>,
			B,
			Functional::TypeDowngrade<C>>
			e;
		static_assert(e.FIRST == 77);
		static_assert(e.SECOND == 34);
	}
	{
		using Trait = Functional::TypeTrait<>;
		static_assert(!Trait::HasTypeFirst::value);
	}

	{
		using Trait = Functional::TypeTrait<
			Functional::TypeUpgrade<67>,
			Functional::TypeUpgrade<42>,
			B,
			Functional::TypeDowngrade<C>>;
		static_assert(Trait::TypeFirst::UNDERLYING == 67);
		static_assert(
			Trait::TypeTraitRemaining::TypeFirst::UNDERLYING ==
			42);
		static_assert(!Trait::IsLessThan<65>::value);
		static_assert(Trait::IsLessThan<69>::value);
		static_assert(Trait::HasTypeFirst::value);
		static_assert(
			Trait::TypeTraitRemaining::HasTypeFirst::value);
		static_assert(Trait::TypeTraitRemaining::
				TypeTraitRemaining::HasTypeFirst::value);
		static_assert(
			Trait::TypeTraitRemaining::TypeTraitRemaining::
				TypeTraitRemaining::HasTypeFirst::value);
		static_assert(!Trait::TypeTraitRemaining::
				TypeTraitRemaining::TypeTraitRemaining::
					TypeTraitRemaining::HasTypeFirst::value);

		static_assert(Functional::TypeTrait<
			Functional::TypeDowngrade<std::pair>>::
				IsTemplateOf<std::pair<int, char>>::value);
		static_assert(
			!Functional::TypeTrait<Functional::TypeDowngrade<
				std::pair>>::IsTemplateOf<std::vector<int>>::value);
		static_assert(Functional::TypeTrait<
			Functional::TypeDowngrade<std::vector>>::
				IsTemplateOf<std::vector<int>>::value);
	}

	return 0;
}

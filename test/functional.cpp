#include <rain.hpp>

using namespace Rain::Functional;

class Type1 {
	public:
	struct Trait {};
};

class Type2 {
	public:
	struct Trait {
		static inline bool constexpr IS_INTEGRAL{true};
		static inline bool constexpr IS_SIGNED{false};
	};
};

template<typename, typename>
class Type3 {};

class Type4 :
	public Type3<TypeUpgrade<4>, TypeUpgrade<true>> {};

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

	static_assert(!TraitType<std::size_t>::IsSigned::VALUE);

	static_assert(!TraitType<Type1>::IsIntegral::VALUE);
	static_assert(!TraitType<Type1>::IsFloatingPoint::VALUE);
	static_assert(!TraitType<Type1>::IsArithmetic::VALUE);
	static_assert(!TraitType<Type1>::IsSigned::VALUE);

	static_assert(TraitType<Type2>::IsIntegral::VALUE);
	static_assert(!TraitType<Type2>::IsFloatingPoint::VALUE);
	static_assert(TraitType<Type2>::IsArithmetic::VALUE);
	static_assert(!TraitType<Type2>::IsSigned::VALUE);

	return 0;
}

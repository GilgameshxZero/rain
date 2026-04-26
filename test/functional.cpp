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

template<std::size_t, bool>
class Type3 {};

class Type4 : public Type3<4, true> {};

int main() {
	// Must include <array>.
	static_assert(TraitType<long long>::IsStdHashable::VALUE);
	static_assert(
		!TraitType<std::array<int, 8>>::IsStdHashable::VALUE);
	static_assert(
		!(TraitTypeTemplate<std::pair>::IsBaseOfTemplate<
			std::array<int, 8>>::VALUE));
	static_assert(
		TraitType<std::array<int, 8>>::IsConstIterable::VALUE);

	// Cannot use std::is_base_of with template base types.
	static_assert(
		TraitTypeTemplate<std::pair>::IsBaseOfTemplate<
			std::pair<int, char>>::VALUE);
	static_assert(
		!TraitTypeTemplate<std::vector>::IsBaseOfTemplate<
			std::pair<int, char>>::VALUE);
	static_assert(
		TraitTypeTemplate<std::vector>::IsBaseOfTemplate<
			std::vector<int>>::VALUE);

	static_assert(!TraitTypeTemplateAuto<
		Type3>::IsBaseOfTemplate<std::vector<int>>::VALUE);
	static_assert(TraitTypeTemplateAuto<
		Type3>::IsBaseOfTemplate<Type4>::VALUE);

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

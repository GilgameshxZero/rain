#include <rain.hpp>

using namespace Rain;
using Rain::Error::releaseAssert;

int main() {
	{
		std::random_device randomDevice;
		std::mt19937_64 generator(randomDevice());
		std::cout
			<< std::uniform_int_distribution<std::uint64_t>()(
					 generator)
			<< std::endl;
	}

	{
		using Type = std::array<long long, 8>;
		using Hash = Random::SplitMixHash<Type>;
		static_assert(
			!Functional::TypeTrait<Type>::IsStdHashable::value &&
			Functional::TypeTrait<Type>::IsConstIterable::value);
		static_assert(
			Functional::TypeTrait<
				long long const>::IsStdHashable::value &&
			sizeof(std::size_t) == 8);
		Hash{}(Type());
	}
	{
		using Type = std::array<long long, 8>;
		std::unordered_map<
			Type,
			std::string,
			Rain::Random::SplitMixHash<Type>>
			X;
		X.insert({{5, 6, 7}, "hi"});
	}

	{
		using TypeInner = std::vector<long long>;
		using Hash = Random::SplitMixHash<TypeInner>;
		static_assert(
			!Functional::TypeTrait<
				TypeInner>::IsStdHashable::value &&
			!Functional::TypeTrait<Functional::TypeDowngrade<
				std::pair>>::IsTemplateBaseOf<TypeInner>::value &&
			Functional::TypeTrait<
				TypeInner>::IsConstIterable::value);
		Hash()(TypeInner());
	}
	{
		using Type = std::vector<long long>;
		std::
			unordered_set<Type, Rain::Random::SplitMixHash<Type>>
				X;
		X.insert({5, 6, 7});
		X.insert({8, 9});
		X.insert({10, 13, 15, 167});
	}

	// {
	// 	// Fails because Type is not a container.
	// 	using Type = std::ifstream;
	// 	std::unordered_set<Type,
	// Rain::Random::SplitMixHash<Type>> X;
	// }

	{
		using TypeInner =
			std::unordered_map<long long, std::string>;
		std::unordered_set<
			TypeInner,
			Rain::Random::SplitMixHash<TypeInner>>
			X;
		X.insert({{1, "hello"}, {2, "world"}});
		releaseAssert(X.size() == 1);
	}

	{
		using Type = std::pair<int, bool>;
		static_assert(
			!Functional::TypeTrait<Type>::IsStdHashable::value &&
			!Functional::TypeTrait<
				Type>::IsConstIterable::value &&
			Functional::TypeTrait<Functional::TypeDowngrade<
				std::pair>>::IsTemplateBaseOf<Type>::value);
		std::
			unordered_set<Type, Rain::Random::SplitMixHash<Type>>
				X;
		X.insert({5, false});
	}

	// Fails because we haven't implemented tuple hash.
	// {
	// 	using Type = std::tuple<int, bool>;
	// 	std::
	// 		unordered_set<Type,
	// Rain::Random::SplitMixHash<Type>> 			X; 	X.insert({5,
	// false});
	// }
	return 0;
}

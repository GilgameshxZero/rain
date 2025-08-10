#include <rain/random.hpp>

#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

int main() {
	{
		using Type = std::array<long long, 8>;
		std::unordered_map<Type, std::string, Rain::Random::SplitMixHash<Type>> X;
		X.insert({{5, 6, 7}, "hi"});
	}
	{
		using Type = std::vector<long long>;
		std::unordered_set<Type, Rain::Random::SplitMixHash<Type>> X;
		X.insert({5, 6, 7});
		X.insert({8, 9});
		X.insert({10, 13, 15, 167});
	}
	// {
	// 	// Fails because Type is not a container.
	// 	using Type = std::ifstream;
	// 	std::unordered_set<Type, Rain::Random::SplitMixHash<Type>> X;
	// }
	{
		using Type = std::unordered_map<long long, std::string>;
		std::unordered_set<Type, Rain::Random::SplitMixHash<Type>> X;
		X.insert({{1, "hello"}, {2, "world"}});
		assert(X.size() == 1);
	}
	{
		using Type = std::pair<int, bool>;
		std::unordered_set<Type, Rain::Random::SplitMixHash<Type>> X;
		X.insert({5, false});
	}
	// Fails because we haven't implemented tuple hash.
	// { std::unordered_set<std::tuple<int, bool>> X; }
	return 0;
}

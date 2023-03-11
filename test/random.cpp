#include <rain/random.hpp>

#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

int main() {
	{
		using Type = std::array<long long, 8>;
		std::unordered_map<Type, std::string, Rain::Random::ContainerHash<Type>> X;
	}
	{
		using Type = std::vector<long long>;
		std::unordered_set<Type, Rain::Random::ContainerHash<Type>> X;
		X.insert({5, 6, 7});
	}
	// {
	// 	// Fails because Type is not a container.
	// 	using Type = std::ifstream;
	// 	std::unordered_set<Type, Rain::Functional::ContainerHash<Type>> X;
	// }
	{
		using Type = std::unordered_map<long long, std::string>;
		std::unordered_set<Type, Rain::Random::ContainerHash<Type>> X;
	}
	{
		std::unordered_set<std::pair<int, bool>, Rain::Random::PairHash<int, bool>>
			X;
		X.insert({5, false});
	}
	// Fails because we haven't implemented tuple hash.
	// { std::unordered_set<std::tuple<int, bool>> X; }
	return 0;
}

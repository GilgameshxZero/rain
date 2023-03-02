#include <rain/functional.hpp>

#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

int main() {
	{
		using Type = std::array<long long, 8>;
		std::unordered_map<Type, std::string, Rain::Functional::ContainerHash<Type>>
			X;
	}
	{
		using Type = std::vector<long long>;
		std::unordered_set<Type, Rain::Functional::ContainerHash<Type>> X;
	}
	// {
	// 	// Fails because Type is not a container.
	// 	using Type = std::ifstream;
	// 	std::unordered_set<Type, Rain::Functional::ContainerHash<Type>> X;
	// }
	{
		using Type = std::unordered_map<long long, std::string>;
		std::unordered_set<Type, Rain::Functional::ContainerHash<Type>> X;
	}
	return 0;
}

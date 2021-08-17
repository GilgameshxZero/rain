// Tests the Least-Recently-Used caching algorithm from
// `rain/algorithm/lru.hpp`.
#include <rain/algorithm/lru.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;

	// General test.
	{
		Rain::Algorithm::LruCache<int, int> cache(2);

		// Assign.
		cache.insertOrAssign(1, 1);
		cache.insertOrAssign(2, 2);
		int res = cache.at(1);
		std::cout << "cache.at(1): " << res << "." << std::endl;
		assert(res == 1);

		// Eviction.
		cache.insertOrAssign(3, 3);
		try {
			res = cache.at(2);
			std::cout << "cache.at(2): " << res << "." << std::endl;
			assert(0);
		} catch (std::out_of_range const &exception) {
			std::cout << exception.what() << std::endl;
		}

		// Eviction & LRU behavior.
		cache.insertOrAssign(4, 4);
		try {
			res = cache.at(1);
			std::cout << "cache.at(1): " << res << "." << std::endl;
			assert(0);
		} catch (std::out_of_range const &exception) {
			std::cout << exception.what() << std::endl;
		}

		res = cache.at(3);
		std::cout << "cache.at(3): " << res << "." << std::endl;
		assert(res == 3);

		res = cache.at(4);
		std::cout << "cache.at(4): " << res << "." << std::endl;
		assert(res == 4);

		// Assign.
		cache.insertOrAssign(4, 5);
		res = cache.at(4);
		std::cout << "cache.at(4): " << cache.at(4) << "." << std::endl;
		assert(res == 5);
	}

	// Copy/move construct test.
	{
		std::vector<std::string> person{"Ben", "Jerry", "Mary", "Thomas", "Kelly"};
		std::vector<std::string> color{"red", "green", "blue", "purple", "orange"};
		Rain::Algorithm::LruCache<std::string, std::string> cache(2);

		// Move-construct a person/color pair.
		cache.insertOrAssign(std::move(person[0]), std::move(color[0]));
		assert(person[0].length() == 0);
		assert(color[0].length() == 0);

		// Move-construct only value argument.
		cache.insertOrAssign(person[1], std::move(color[1]));
		assert(!person[1].empty());
		assert(color[1].length() == 0);

		// Check things are valid.
		assert(cache.at("Ben") == "red");
		assert(cache.at("Jerry") == "green");

		// Ben is now LRU.
		assert(cache.find("Ben") != cache.end());

		// Adding a third value makes find return end.
		cache.insertOrAssign(std::move(person[2]), std::move(color[2]));
		assert(cache.find("Jerry") == cache.end());

		// Iteration works as expected.
		for (auto const &it : cache) {
			std::cout << it.first << ": " << it.second << std::endl;
		}
	}

	return 0;
}

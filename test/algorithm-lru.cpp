// Tests the Least-Recently-Used caching algorithm from
// `rain/algorithm/lru.hpp`.
#include <rain/algorithm/lru.hpp>

#include <cassert>
#include <iostream>

int main() {
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

	return 0;
}

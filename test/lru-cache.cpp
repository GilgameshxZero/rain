#include <rain/algorithm/lru.hpp>

#include <iostream>

int main() {
	Rain::Algorithm::LRUCache<int, int> cache(2);
	cache.insert_or_assign(1, 1);
	cache.insert_or_assign(2, 2);
	std::cout << "cache.at(1): " << cache.at(1) << std::endl;
	cache.insert_or_assign(3, 3);
	try {
		std::cout << "cache.at(2): " << cache.at(2) << std::endl;
	} catch (...) {
		std::cout << "error" << std::endl;
	}
	cache.insert_or_assign(4, 4);
	try {
		std::cout << "cache.at(1): " << cache.at(1) << std::endl;
	} catch (...) {
		std::cout << "error" << std::endl;
	}
	std::cout << "cache.at(3): " << cache.at(3) << std::endl;
	std::cout << "cache.at(4): " << cache.at(4) << std::endl;
	cache.insert_or_assign(4, 5);
	std::cout << "cache.at(4): " << cache.at(4) << std::endl;

	return 0;
}

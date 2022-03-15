#include <rain/algorithm/algorithm.hpp>
#include <rain/algorithm/mod-ring.hpp>

#include <cassert>
#include <iostream>

int main() {
	{
		Rain::Algorithm::ModRing<long long, 998244353> x, y(100);
		Rain::Algorithm::ModRing<long long, 1000000009> z;
		assert(sizeof(long long) >= 8);
		x += 5;
		assert(x == 5);
		x += y;
		assert(x == 105);
		x++;
		assert(x == 106);
		x -= 6;
		assert(x == y);
		x -= 100;
		assert(x == 0);
		x -= -35;
		assert(x == 998244388);
		x += -50;
		assert(x == 998244338);
		// assert(x == z); // Cannot compare between different mods.
		x = 2;
		x /= 2;
		assert(x == 1);
		x = 343;
		x /= 7;
		x *= 14;
		assert(x == 686);
		z = 1000000009;
		assert(z == 0);
		assert(z == 1000000009);
		assert(z == 2000000018);
		z--;
		assert(z == -1);
		assert(z == 2000000017);
		x = 31;
		assert(y == 100);
		x *= 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 * y * 39486758 * y *
			3049857272LL;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		assert(x / y / 10000 / 3049857272LL / 31 == 39486758);
		assert(x / 10000 / 3049857272LL / 39486758 / 31 == y);
	}

	{
		Rain::Algorithm::ModRing<std::size_t, 998244353> x, y(100);
		Rain::Algorithm::ModRing<std::size_t, 1000000009> z;
		// Make sure this is true!
		assert(sizeof(std::size_t) >= 8);
		x += 5;
		assert(x == 5);
		x += y;
		assert(x == 105);
		x++;
		assert(x == 106);
		x -= 6;
		assert(x == y);
		x -= 100;
		assert(x == 0);
		x -= -35;
		assert(x == 998244388);
		x += -50;
		assert(x == 998244338);
		// assert(x == z); // Cannot compare between different mods.
		x = 2;
		x /= 2;
		assert(x == 1);
		x = 343;
		x /= 7;
		x *= 14;
		assert(x == 686);
		z = 1000000009;
		assert(z == 0);
		assert(z == 1000000009);
		assert(z == 2000000018);
		z--;
		assert(z == -1);
		assert(z == 2000000017);
		x = 31;
		assert(y == 100);
		x *= 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 * y * 39486758 * y *
			3049857272LL;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		assert(x / y / 10000 / 3049857272LL / 31 == 39486758);
		assert(x / 10000 / 3049857272LL / 39486758 / 31 == y);
	}

	Rain::Algorithm::ModRing<uint64_t, 998244353> v{99};
	assert(sizeof(uint64_t) == 8);
	assert((v + 504957) / 3 == 168352);
	assert(
		v - 2038 ==
		998242414);	 // Must be positive comparison, since std::size_t is positive.
	assert(v - 2038 == -1939);
	assert(v - 998244355 == 97);
	std::cout << "v is " << v << '.' << std::endl;

	auto res = Rain::Algorithm::ModRing<std::size_t, 23>::power(5, 4);
	assert(res == 625 % 23);

	// Fibonacci tests.
	assert(
		(Rain::Algorithm::fibonacciNumber<
			 Rain::Algorithm::ModRing<std::size_t, 988244353>>(1000000000) ==
		 910643820));

	return 0;
}

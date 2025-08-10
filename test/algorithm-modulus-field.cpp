#include <rain/algorithm/fibonacci.hpp>
#include <rain/algorithm/modulus-field.hpp>

#include <cassert>
#include <iostream>
#include <unordered_set>

int main() {
	{
		assert(!(Rain::Algorithm::isDerivedFromModulusRing<int>()).value);
		assert((Rain::Algorithm::isDerivedFromModulusRing<
							Rain::Algorithm::ModulusField<long long, 17>>())
						 .value);
		assert((2 * Rain::Algorithm::ModulusField<int, 7>{3}).value == 6);
	}

	{
		using PMR1 = Rain::Algorithm::ModulusField<long long, 998244353>;
		using PMR2 = Rain::Algorithm::ModulusField<long long, 1000000009>;

		PMR1 x, y(100);
		PMR2 z;
		assert(sizeof(long long) >= 8);
		x += 5;
		assert(x == 5);
		assert(x != 6);
		assert(x == x);
		assert(x != y);
		assert(x < y);
		assert(x <= y);
		assert(y > x);
		assert(y >= x);
		assert(x < 6);
		assert(x <= 5);

		bool bx{x};
		std::size_t sx{x};
		int ix{x};
		assert(bx);
		assert(sx == 5);
		assert(ix == 5);
		// This is not an explicit cast and should fail.
		// ix = x;
		// This is, deceptively, still an explicit cast and will compile.
		std::vector<int> vx(x);
		assert(vx.size() == 5);

		x += y.value;
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
		z = -3;
		assert(z == -3);
		assert(z == 1000000006);
		x = 31;
		assert(y == 100);
		x *= (2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 * y * 39486758 * y *
					3049857272LL)
					 .value;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		assert(x / y / 10000 / 3049857272LL / 31 == 39486758);
		assert(x / 10000 / 3049857272LL / 39486758 / 31 == y);
		x = z;
		x = y;

		// Factorials and chooses.
		PMR1::precomputeFactorials(4096);
		assert(PMR1::factorials[5] == 120);
		assert(PMR1::factorials[1000] == 421678599);
		assert(PMR1(6).choose(2) == 15);
		assert(PMR1(1000).choose(45) == 991398900);
	}

	{
		Rain::Algorithm::ModulusField<std::size_t, 998244353> x, y(100);
		Rain::Algorithm::ModulusField<std::size_t, 1000000009> z;
		// Make sure this is true!
		assert(sizeof(std::size_t) >= 8);
		x += 5;
		assert(x == 5);
		x += y.value;
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
		x *= (2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 * y * 39486758 * y *
					3049857272LL)
					 .value;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		assert(x / y / 10000 / 3049857272LL / 31 == 39486758);
		assert(x / 10000 / 3049857272LL / 39486758 / 31 == y);
	}

	{
		Rain::Algorithm::ModulusField<uint64_t, 998244353> v{99};
		assert(sizeof(uint64_t) == 8);
		assert((v + 504957) / 3 == 168352);
		assert(v - 2038 == 998242414);
		assert(v - 2038 == -1939);
		assert(v - 998244355 == 97);
		std::cout << "v is " << v << '.' << std::endl;

		auto res{Rain::Algorithm::ModulusField<std::size_t, 23>(5).power(4)};
		assert(res == 625 % 23);
	}

	// Fibonacci tests.
	{
		assert(
			(Rain::Algorithm::fibonacciNumber<
				 Rain::Algorithm::ModulusField<std::size_t, 988244353>>(1000000000) ==
			 910643820));
	}

	// Runtime modulus.
	{
		Rain::Algorithm::ModulusField<long long> x(23);
		x = 3;
		assert(x == 3);
		assert(x == -20);
		assert(x == 26);

		auto y{x + 11};
		assert(y == 14);

		Rain::Algorithm::ModulusField<long long> z(13, 1);
		assert(z == 1);
		assert(z * 14 == 14);

		// Assignment with differing moduli always prefers the lvalue modulus.
		assert(y == 14);
		assert(19 == 5 + y);
		assert(y.MODULUS > 14);
		assert(z.MODULUS == 13);
		z = y;
		assert(z == 1);
		z = 5;
		assert(z == 5);

		z = y.value;
		assert(z.value == 1);
		x = y;
		assert(x == 14);

		assert(24 + x == 15);
		assert(24 - x == 10);
		assert(24 * x == 14);
		assert(24 / x == 5);
	}

	{
		using MF = Rain::Algorithm::ModulusField<std::size_t, 13>;
		MF a, b, c, d, e;

		// Constructing with a large negative integer should still wrap around
		// correctly.
		a = -28;
		assert(a == 11);

		// Making sure const versions of functions work as expected.
		c = a / 15;
		MF const f(15), g(a);
		d = g / f;
		assert(c == d);
		c = a + 15;
		d = g + f;
		assert(c == d);
		assert(a == a);
		assert(a == g);
		assert(g == a);
		assert(g == g);

		a = 1;
		assert(a > b);
		assert(a >= b);
		assert(b == c);
	}

	{
		using MF = Rain::Algorithm::ModulusField<std::size_t, 13>;

		// Is a hashable type.
		std::unordered_set<MF> S;
		S.insert({24});
	}

	{
		using MR = Rain::Algorithm::ModulusRing<long long, 14>;
		using MF = Rain::Algorithm::ModulusField<long long, 7>;
		MR x{5};
		MF y{1};

		x *= 5;
		assert(x == 11);
		assert(x == -3);
		assert(y == -6);
		y /= 4;
		assert(y == 2);
		y = x;
		assert(y == 4);
		y = -5;
		assert(y == 2);
		x = y;
		assert(x == 2);
	}

	return 0;
}

#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	{
		releaseAssert(!(Rain::Algorithm::ModulusRingBase<>::
											isDerivedFromModulusRing<int>())
										 .value);
		releaseAssert(
			(Rain::Algorithm::ModulusRingBase<>::
				 isDerivedFromModulusRing<
					 Rain::Algorithm::ModulusField<long long, 17>>())
				.value);
		releaseAssert(
			(2 * Rain::Algorithm::ModulusField<int, 7>{3})
				.value == 6);
	}

	{
		using PMR1 =
			Rain::Algorithm::ModulusField<long long, 998244353>;
		using PMR2 =
			Rain::Algorithm::ModulusField<long long, 1000000009>;

		PMR1 x, y(100);
		PMR2 z;
		releaseAssert(sizeof(long long) >= 8);
		x += 5;
		releaseAssert(x == 5);
		releaseAssert(x != 6);
		releaseAssert(x == x);
		releaseAssert(x != y);
		releaseAssert(x < y);
		releaseAssert(x <= y);
		releaseAssert(y > x);
		releaseAssert(y >= x);
		releaseAssert(x < 6);
		releaseAssert(x <= 5);

		bool bx{x};
		std::size_t sx{x};
		int ix{x};
		releaseAssert(bx);
		releaseAssert(sx == 5);
		releaseAssert(ix == 5);
		// This is not an explicit cast and should fail.
		// ix = x;
		// This is, deceptively, still an explicit cast and will
		// compile.
		std::vector<int> vx(x);
		releaseAssert(vx.size() == 5);

		x += y.value;
		releaseAssert(x == 105);
		x++;
		releaseAssert(x == 106);
		x -= 6;
		releaseAssert(x == y);
		x -= 100;
		releaseAssert(x == 0);
		x -= -35;
		releaseAssert(x == 998244388);
		x += -50;
		releaseAssert(x == 998244338);
		// releaseAssert(x == z); // Cannot compare between
		// different mods.
		x = 2;
		x /= 2;
		releaseAssert(x == 1);
		x = 343;
		x /= 7;
		x *= 14;
		releaseAssert(x == 686);
		z = 1000000009;
		releaseAssert(z == 0);
		releaseAssert(z == 1000000009);
		releaseAssert(z == 2000000018);
		z--;
		releaseAssert(z == -1);
		releaseAssert(z == 2000000017);
		z = -3;
		releaseAssert(z == -3);
		releaseAssert(z == 1000000006);
		x = 31;
		releaseAssert(y == 100);
		x *= (2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 *
					y * 39486758 * y * 3049857272LL)
					 .value;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		releaseAssert(
			x / y / 10000 / 3049857272LL / 31 == 39486758);
		releaseAssert(
			x / 10000 / 3049857272LL / 39486758 / 31 == y);
		x = z;
		x = y;

		// Factorials and chooses.
		PMR1::precomputeFactorials(4096);
		releaseAssert(PMR1::factorials[5] == 120);
		releaseAssert(PMR1::factorials[1000] == 421678599);
		releaseAssert(PMR1(6).choose(2) == 15);
		releaseAssert(PMR1(1000).choose(45) == 991398900);
	}

	{
		Rain::Algorithm::ModulusField<std::size_t, 998244353> x,
			y(100);
		Rain::Algorithm::ModulusField<std::size_t, 1000000009>
			z;
		// Make sure this is true!
		releaseAssert(sizeof(std::size_t) >= 8);
		x += 5;
		releaseAssert(x == 5);
		x += y.value;
		releaseAssert(x == 105);
		x++;
		releaseAssert(x == 106);
		x -= 6;
		releaseAssert(x == y);
		x -= 100;
		releaseAssert(x == 0);
		x -= -35;
		releaseAssert(x == 998244388);
		x += -50;
		releaseAssert(x == 998244338);
		// releaseAssert(x == z); // Cannot compare between
		// different mods.
		x = 2;
		x /= 2;
		releaseAssert(x == 1);
		x = 343;
		x /= 7;
		x *= 14;
		releaseAssert(x == 686);
		z = 1000000009;
		releaseAssert(z == 0);
		releaseAssert(z == 1000000009);
		releaseAssert(z == 2000000018);
		z--;
		releaseAssert(z == -1);
		releaseAssert(z == 2000000017);
		x = 31;
		releaseAssert(y == 100);
		x *= (2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31 *
					y * 39486758 * y * 3049857272LL)
					 .value;
		y = 2LL * 3 * 5 * 7 * 11 * 13 * 17 * 23 * 29 * 31;
		releaseAssert(
			x / y / 10000 / 3049857272LL / 31 == 39486758);
		releaseAssert(
			x / 10000 / 3049857272LL / 39486758 / 31 == y);
	}

	{
		Rain::Algorithm::ModulusField<uint64_t, 998244353> v{
			99};
		releaseAssert(sizeof(uint64_t) == 8);
		releaseAssert((v + 504957) / 3 == 168352);
		releaseAssert(v - 2038 == 998242414);
		releaseAssert(v - 2038 == -1939);
		releaseAssert(v - 998244355 == 97);
		std::cout << "v is " << v << '.' << std::endl;

		auto res{
			Rain::Algorithm::ModulusField<std::size_t, 23>(5)
				.power(4)};
		releaseAssert(res == 625 % 23);
	}

	// Fibonacci tests.
	{
		releaseAssert(
			(Rain::Algorithm::fibonacciNumber<
				 Rain::Algorithm::
					 ModulusField<std::size_t, 988244353>>(
				 1000000000) == 910643820));
	}

	// Runtime modulus.
	{
		Rain::Algorithm::ModulusField<long long> x(23);
		x = 3;
		releaseAssert(x == 3);
		releaseAssert(x == -20);
		releaseAssert(x == 26);

		auto y{x + 11};
		releaseAssert(y == 14);

		Rain::Algorithm::ModulusField<long long> z(13, 1);
		releaseAssert(z == 1);
		releaseAssert(z * 14 == 14);

		// Assignment with differing moduli always prefers the
		// lvalue modulus.
		releaseAssert(y == 14);
		releaseAssert(19 == 5 + y);
		releaseAssert(y.MODULUS > 14);
		releaseAssert(z.MODULUS == 13);
		z = y;
		releaseAssert(z == 1);
		z = 5;
		releaseAssert(z == 5);

		z = y.value;
		releaseAssert(z.value == 1);
		x = y;
		releaseAssert(x == 14);

		releaseAssert(24 + x == 15);
		releaseAssert(24 - x == 10);
		releaseAssert(24 * x == 14);
		releaseAssert(24 / x == 5);
	}

	{
		using MF =
			Rain::Algorithm::ModulusField<std::size_t, 13>;
		MF a, b, c, d, e;

		// Constructing with a large negative integer should
		// still wrap around correctly.
		a = -28;
		releaseAssert(a == 11);

		// Making sure const versions of functions work as
		// expected.
		c = a / 15;
		MF const f(15), g(a);
		d = g / f;
		releaseAssert(c == d);
		c = a + 15;
		d = g + f;
		releaseAssert(c == d);
		releaseAssert(a == a);
		releaseAssert(a == g);
		releaseAssert(g == a);
		releaseAssert(g == g);

		a = 1;
		releaseAssert(a > b);
		releaseAssert(a >= b);
		releaseAssert(b == c);
	}

	{
		using MF =
			Rain::Algorithm::ModulusField<std::size_t, 13>;

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
		releaseAssert(x == 11);
		releaseAssert(x == -3);
		releaseAssert(y == -6);
		y /= 4;
		releaseAssert(y == 2);
		y = x;
		releaseAssert(y == 4);
		y = -5;
		releaseAssert(y == 2);
		x = y;
		releaseAssert(x == 2);
	}

	return 0;
}

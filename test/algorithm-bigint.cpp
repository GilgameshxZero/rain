#include <rain/algorithm/bigint.hpp>
#include <rain/algorithm/bit-manipulators.hpp>

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_set>

int main() {
	// 64-bits.
	{
		Rain::Algorithm::BigIntSigned<6> x;
		assert(sizeof(x) == 8);
	}

	// 128-bits.
	{
		Rain::Algorithm::BigIntSigned<7> x;
		assert(sizeof(x) == 16);
	}

	// 256-bits.
	{
		Rain::Algorithm::BigIntSigned<8> x;
		assert(sizeof(x) == 32);
	}

	{
		Rain::Algorithm::BigIntSigned<6> x(42, 69);
		x += 1000000009;
		x += 1000000009;
		x += 1000000009;
		assert(x.low == 3000000069);
		assert(x.high == 69);
		x += 1000000009;
		x += 1000000009;
		x += 1000000009;
		assert(x.low == 1705032800);
		assert(x.high == 70);

		x -= 1705032800;
		assert(x.low == 0);
		assert(x.high == 70);
		x--;
		assert(x.low == 4294967295);
		assert(x.high == 69);

		Rain::Algorithm::BigIntSigned<6> y(1000000007), z(1000000009);
		x = y * z;
		assert(x.low == 1628479551);
		assert(x.high == 232830647);
		y *= -1;
		assert(y.low == 3294967289);
		assert(y.high == -1);
		x = y * z;
		assert(x.low == 2666487745);
		assert(x.high == -232830648);
		x *= -2;
		assert(x.low == 3256959102);
		assert(x.high == 465661294);
		x *= 1000000000000;
		assert(x.low == 4248690688);
		assert(x.high == 1176504104);
		x = -1000000000000;
		assert(x.low == 727379968);
		assert(x.high == -233);
	}

	{
		Rain::Algorithm::BigIntSigned<6> x(42, 69);
		x >>= 3;
		assert(x == 37044092933);
		x <<= 3;
		assert(x == 296352743464);
	}

	{
		Rain::Algorithm::BigIntSigned<6> x(42, 69);
		x /= 5;
		assert(x == 59270548693);
		x *= 1000000;
		assert(x == 59270548693000000);
		x /= 1000000009;
		assert(x == 59270548);
	}

	{
		Rain::Algorithm::BigIntSigned<7> x(18446744073709551557ULL);
		x /= 998244353;
		assert(x == 18479187002);
		// Need literal here because constructors cannot handle unsigned integers
		// too well.
		x = 18446744073709551557ULL;
		x *= 998244353;
		assert(x.low.low == 1233125317);
		assert(x.low.high == 4294967282);
		assert(x.high.low == 998244352);
		assert(x.high.high == 0);
	}

	{
		Rain::Algorithm::BigIntSigned<7> x(18446744073709551557ULL), y(5);
		x = 12 + y + 100 + y + y + 3;
		assert(x == 130);

		Rain::Algorithm::BigIntSigned<7> const Z(18446744073709551557ULL);
		std::size_t lsb{leastSignificant1BitIdx(Z - 1)};
		assert(lsb == 2);
	}

	{
		Rain::Algorithm::BigIntSigned<7> x(18446744073709551557ULL);
		x *= 1000000009;

		std::stringstream ss;
		ss << x;
		assert(ss.str() == "18446744239730248220385964013");
		x = 0;
		assert(x == 0);
		ss >> x;
		x /= 18446744073709551557ULL;
		assert(x == 1000000009);
	}

	{
		using BI = Rain::Algorithm::BigIntSigned<10>;
		BI a, b, c;
		a = 15;
		BI const d(a), e(20);
		assert(a == d);
		b = e;
		assert(b == e);
		c = a + b;
		assert(c == d + e);
	}

	// Hashable.
	{
		std::unordered_set<Rain::Algorithm::BigIntSigned<10>> S;
		S.insert({5});
	}

	return 0;
}

#include <rain.hpp>

using Rain::Error::releaseAssert;
using namespace Rain::Algorithm;

int main() {
	// 64-bits.
	{
		BigIntSigned<6> x;
		releaseAssert(sizeof(x) == 8);
	}

	// 128-bits.
	{
		BigIntSigned<7> x;
		releaseAssert(sizeof(x) == 16);
	}

	// 256-bits.
	{
		BigIntSigned<8> x;
		releaseAssert(sizeof(x) == 32);
	}

	{
		BigIntSigned<6> x(42, 69);
		x += 1000000009;
		x += 1000000009;
		x += 1000000009;
		releaseAssert(x.low == 3000000069);
		releaseAssert(x.high == 69);
		x += 1000000009;
		x += 1000000009;
		x += 1000000009;
		releaseAssert(x.low == 1705032800);
		releaseAssert(x.high == 70);

		x -= 1705032800;
		releaseAssert(x.low == 0);
		releaseAssert(x.high == 70);
		x--;
		releaseAssert(x.low == 4294967295);
		releaseAssert(x.high == 69);

		BigIntSigned<6> y(1000000007), z(1000000009);
		x = y * z;
		releaseAssert(x.low == 1628479551);
		releaseAssert(x.high == 232830647);
		y *= -1;
		releaseAssert(y.low == 3294967289);
		releaseAssert(y.high == -1);
		x = y * z;
		releaseAssert(x.low == 2666487745);
		releaseAssert(x.high == -232830648);
		x *= -2;
		releaseAssert(x.low == 3256959102);
		releaseAssert(x.high == 465661294);
		x *= 1000000000000;
		releaseAssert(x.low == 4248690688);
		releaseAssert(x.high == 1176504104);
		x = -1000000000000;
		releaseAssert(x.low == 727379968);
		releaseAssert(x.high == -233);
	}

	{
		BigIntSigned<6> x(42, 69);
		x >>= 3;
		releaseAssert(x == 37044092933);
		x <<= 3;
		releaseAssert(x == 296352743464);
	}

	{
		BigIntSigned<6> x(42, 69);
		x /= 5;
		releaseAssert(x == 59270548693);
		x *= 1000000;
		releaseAssert(x == 59270548693000000);
		x /= 1000000009;
		releaseAssert(x == 59270548);
	}

	{
		BigIntSigned<7> x(18446744073709551557ULL);
		x /= 998244353;
		releaseAssert(x == 18479187002);
		// Need literal here because constructors cannot handle
		// unsigned integers too well.
		x = 18446744073709551557ULL;
		x *= 998244353;
		releaseAssert(x.low.low == 1233125317);
		releaseAssert(x.low.high == 4294967282);
		releaseAssert(x.high.low == 998244352);
		releaseAssert(x.high.high == 0);
	}

	{
		BigIntSigned<7> x(18446744073709551557ULL), y(5);
		x = 12 + y + 100 + y + y + 3;
		releaseAssert(x == 130);

		BigIntSigned<7> const Z(18446744073709551557ULL);
		std::size_t lsb{leastSignificant1BitIdx(Z - 1)};
		releaseAssert(lsb == 2);
	}

	{
		BigIntSigned<7> x(18446744073709551557ULL);
		x *= 1000000009;

		std::stringstream ss;
		ss << x;
		releaseAssert(
			ss.str() == "18446744239730248220385964013");
		x = 0;
		releaseAssert(x == 0);
		ss >> x;
		x /= 18446744073709551557ULL;
		releaseAssert(x == 1000000009);
	}

	{
		using BI = BigIntSigned<10>;
		BI a, b, c;
		a = 15;
		BI const d(a), e(20);
		releaseAssert(a == d);
		b = e;
		releaseAssert(b == e);
		c = a + b;
		releaseAssert(c == d + e);
	}

	// Hashable.
	{
		std::unordered_set<BigIntSigned<10>> S;
		S.insert({5});
	}

	// TODO: Automatic conversions always use larger, signed
	// type.
	// {
	// 	releaseAssert(
	// 		BigIntUnsigned<6>{5} > BigIntSigned<6>{-99});
	// }

	return 0;
}

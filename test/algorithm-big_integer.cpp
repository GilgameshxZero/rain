#include <rain.hpp>

using Rain::Error::releaseAssert;
using namespace Rain::Algorithm;
using namespace std;

int main() {
	// `BigIntegerCommon`.
	{
		static_assert(is_same_v<
			BigIntegerCommon<int, long long>::type,
			BigIntegerSigned<6>>);
		static_assert(is_same_v<
			BigIntegerCommon<int, unsigned long long>::type,
			BigIntegerSigned<7>>);
		static_assert(is_same_v<
			BigIntegerCommon<unsigned int, long long>::type,
			BigIntegerSigned<6>>);
		static_assert(is_same_v<
			BigIntegerCommon<unsigned int, unsigned long long>::
				type,
			BigIntegerUnsigned<6>>);
		static_assert(is_same_v<
			BigIntegerCommon<int, unsigned char>::type,
			BigIntegerSigned<5>>);
		// BigIntegerSigned<LOG_BITS> for LOG_BITS < 5 is not
		// well defined, but still exists as a type.
		static_assert(is_same_v<
			BigIntegerCommon<short, unsigned char>::type,
			BigIntegerSigned<4>>);
	}

	// Sizing.
	{
		releaseAssert(sizeof(BigIntegerSigned<6>) == 8);
		releaseAssert(sizeof(BigIntegerSigned<7>) == 16);
		releaseAssert(sizeof(BigIntegerSigned<8>) == 32);
	}

	// `BigInt<5>` cast & constructors, comparators.
	{
		// Case 1.
		releaseAssert(
			static_cast<unsigned long long>(
				BigIntegerSigned<5>(97)) == 97ULL);
		releaseAssert(
			static_cast<unsigned long long>(
				BigIntegerSigned<5>(-97)) == ULLONG_MAX - 96ULL);
		// Case 2, 3.
		releaseAssert(
			static_cast<int>(BigIntegerUnsigned<5>(97)) == 97);
		releaseAssert(
			static_cast<int>(BigIntegerUnsigned<5>(UINT_MAX)) ==
			INT_MAX);
		releaseAssert(
			static_cast<unsigned int>(
				BigIntegerSigned<5>(INT_MIN)) == UINT_MAX / 2 + 1);
		releaseAssert(
			static_cast<unsigned int>(BigIntegerSigned<5>(
				INT_MIN + 163)) == UINT_MAX / 2 + 164);
		// Case 4.
		releaseAssert(
			static_cast<unsigned char>(
				BigIntegerUnsigned<5>(97)) == 'a');
		// MSVC does not support compound literals (`(unsigned
		// char){243}`).
		releaseAssert(
			static_cast<std::uint8_t>(BigIntegerSigned<5>(-13)) ==
			std::uint8_t{243});
		// Case 5.
		releaseAssert(
			static_cast<char>(BigIntegerUnsigned<5>(97)) == 'a');
		releaseAssert(
			static_cast<char>(BigIntegerSigned<5>(-260)) ==
			char{-4});
		releaseAssert(
			static_cast<char>(BigIntegerSigned<5>(-13)) ==
			char{-13});

		// Comparators.
		releaseAssert(BigIntegerSigned<5>(10) == 10);
		releaseAssert(BigIntegerSigned<5>(10) >= -5);
		releaseAssert(BigIntegerSigned<5>(-99) < 11ULL);
	}

	// BigInteger<5> arithmetic.
	{
		auto z{BigIntegerSigned<5>::addWithOverflow(
			BigIntegerSigned<5>(10), BigIntegerSigned<5>(-23))};
		releaseAssert(z.first == -1);
		releaseAssert(z.second == UINT_MAX - 12);
		auto x{BigIntegerUnsigned<5>::addWithOverflow(
			BigIntegerUnsigned<5>(UINT_MAX),
			BigIntegerUnsigned<5>(7))};
		releaseAssert(x.first == 1);
		releaseAssert(x.second == 6);
		auto y{BigIntegerSigned<5>(INT_MAX) + 3};
		releaseAssert(y == INT_MIN + 2);
		y = BigIntegerSigned<5>(INT_MIN) + -1;
		releaseAssert(y == INT_MAX);

		z = BigIntegerSigned<5>::subtractWithOverflow(
			BigIntegerSigned<5>(10), BigIntegerSigned<5>(23));
		releaseAssert(z.first == -1);
		releaseAssert(z.second == UINT_MAX - 12);
		x = BigIntegerUnsigned<5>::subtractWithOverflow(
			BigIntegerUnsigned<5>(7),
			BigIntegerUnsigned<5>(UINT_MAX));
		releaseAssert(x.first == UINT_MAX);
		releaseAssert(x.second == 8);
		y = BigIntegerSigned<5>(INT_MAX) - -3;
		releaseAssert(y == INT_MIN + 2);
		y = BigIntegerSigned<5>(INT_MIN) - 1;
		releaseAssert(y == INT_MAX);

		z = BigIntegerSigned<5>::multiplyWithOverflow(
			BigIntegerSigned<5>(42069),
			BigIntegerSigned<5>(-1000000000));
		releaseAssert(
			BigIntegerSigned<6>(z.first, z.second) ==
			(42069LL * -1000000000LL));
		releaseAssert(z.first == -9795);
		releaseAssert(z.second == 204664320);
		x = BigIntegerUnsigned<5>::multiplyWithOverflow(
			BigIntegerUnsigned<5>(42069),
			BigIntegerUnsigned<5>(-1000000000));
		// This is not the equivalent of multiplying them
		// in-range, because of modulus effects.
		releaseAssert(x.first == 32274);
		releaseAssert(x.second == 204664320);
		// Multiplying with signed will be as expected.
		z = BigIntegerUnsigned<5>::multiplyWithOverflow(
			BigIntegerUnsigned<5>(42069),
			BigIntegerSigned<5>(-1000000000));
		releaseAssert(
			BigIntegerSigned<6>(z.first, z.second) ==
			(42069LL * -1000000000LL));
		releaseAssert(z.first == -9795);
		releaseAssert(z.second == 204664320);
		y = BigIntegerSigned<5>(INT_MAX) * -3;
		releaseAssert(
			y ==
			BigIntegerSigned<5>() - BigIntegerSigned<5>(INT_MAX) -
				BigIntegerSigned<5>(INT_MAX) -
				BigIntegerSigned<5>(INT_MAX));
		releaseAssert(y == INT_MIN + 3);
		y = BigIntegerSigned<5>(INT_MAX) * INT_MAX;
		releaseAssert(y == 1);

		auto w{BigIntegerSigned<5>::divideWithRemainder(
			BigIntegerSigned<5>(-1000000009),
			BigIntegerSigned<5>(37))};
		releaseAssert(w.first == -10);
		releaseAssert(w.second == -27027027);
		y = INT_MAX;
		releaseAssert(y / -9999 == -214769);
		releaseAssert(y % -9999 == 8416);
	}

	// Hashable.
	{
		std::unordered_set<BigIntegerSigned<10>> S;
		S.insert(BigIntegerSigned<10>(5));
	}

	// Binary.
	{
		BigIntegerSigned<7> a(5);
		a <<= 3;
		releaseAssert(a == 40);
		a >>= -1;
		releaseAssert(a == 80);
		a >>= 5;
		releaseAssert(a == 2);
		releaseAssert(static_cast<int>(a) == 2);
		a = -6;
		releaseAssert(static_cast<int>(a) == -6);
		a <<= -1;
		releaseAssert(a == -3);
		a <<= 5;
		releaseAssert(a == -96);
		a >>= 500;
		releaseAssert(a == -1);

		a = 18446744073709551557ULL;
		for (std::size_t i{6}; i < 64; i++) {
			releaseAssert(
				static_cast<bool>(
					a & (BigIntegerSigned<7>(1) << i)));
		}
		// 111...111100000 (last 1 is at the 2^31 place).
		a = INT_MIN;
		a <<= 4;
		a >>= 33;
		releaseAssert(a == -4);
	}

	// Arithmetic.
	{
		auto z{BigIntegerSigned<6>::addWithOverflow(
			BigIntegerSigned<6>(10), BigIntegerSigned<6>(-23))};
		releaseAssert(z.first == -1);
		releaseAssert(z.second == ULLONG_MAX - 12);
		auto x{BigIntegerUnsigned<6>::addWithOverflow(
			BigIntegerUnsigned<6>(ULLONG_MAX),
			BigIntegerUnsigned<6>(7))};
		releaseAssert(x.first == 1);
		releaseAssert(x.second == 6);
		auto y{BigIntegerSigned<6>(LLONG_MAX) + 3};
		releaseAssert(y == LLONG_MIN + 2);
		y = BigIntegerSigned<6>(LLONG_MIN) + -1;
		releaseAssert(y == LLONG_MAX);

		z = BigIntegerSigned<6>::subtractWithOverflow(
			BigIntegerSigned<6>(10), BigIntegerSigned<6>(23));
		releaseAssert(z.first == -1);
		releaseAssert(z.second == ULLONG_MAX - 12);
		x = BigIntegerUnsigned<6>::subtractWithOverflow(
			BigIntegerUnsigned<6>(7),
			BigIntegerUnsigned<6>(ULLONG_MAX));
		releaseAssert(x.first == ULLONG_MAX);
		releaseAssert(x.second == 8);
		z = BigIntegerSigned<6>::subtractWithOverflow(
			BigIntegerSigned<6>(LLONG_MIN + 7),
			BigIntegerSigned<6>(LLONG_MAX));
		releaseAssert(z.first == -1);
		releaseAssert(z.second == 8);
		y = BigIntegerSigned<6>(LLONG_MAX) - -3;
		releaseAssert(y == LLONG_MIN + 2);
		y = BigIntegerSigned<6>(LLONG_MIN) - 1;
		releaseAssert(y == LLONG_MAX);
		y = BigIntegerSigned<6>(LLONG_MIN);
		y = -y;
		releaseAssert(y == LLONG_MIN);

		z = BigIntegerUnsigned<6>::multiplyWithOverflow(
			BigIntegerUnsigned<6>(42069999999LL),
			BigIntegerSigned<6>(-1000000000));
		releaseAssert(z.first.high == -1);
		releaseAssert(z.first.low == 4294967293);
		releaseAssert(z.second.high == 3089716709);
		releaseAssert(z.second.low == 3068905984);
		y = BigIntegerSigned<6>(LLONG_MAX) * -3;
		releaseAssert(y == LLONG_MIN + 3);
		y = BigIntegerSigned<6>(LLONG_MAX) * INT_MAX;
		releaseAssert(y == 9223372034707292161LL);

		auto w{BigIntegerSigned<6>::divideWithRemainder(
			BigIntegerSigned<6>(-1000000009),
			BigIntegerSigned<6>(37))};
		releaseAssert(w.first == -10);
		releaseAssert(w.second == -27027027);
		y = INT_MAX;
		releaseAssert(y / -9999 == -214769);
		releaseAssert(y % -9999 == 8416);
	}

	// More division.
	{
		auto z{BigIntegerSigned<12>(1)};
		z = z * 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23 * 29 *
			31 * 37 * 41 * 43 * 47 * 53 * 59 * 61 * 67 * 71 * 73 *
			79 * 83 * 89 * 97;
		cout << z << endl;
		releaseAssert(z % 101 == 28);
		z /= 101;
		{
			stringstream ss;
			ss << z;
			releaseAssert(
				ss.str() == "22827405583618994304486159874571842");
		}
		z *= 101;
		cout << z << endl;
		{
			stringstream ss;
			ss << z;
			releaseAssert(
				ss.str() ==
				"2305567963945518424753102147331756042");
		}
	}

	// Misc.
	{
		BigIntegerSigned<6> x(
			BigIntegerSigned<5>(69), BigIntegerUnsigned<5>(42));
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

		BigIntegerSigned<6> y(1000000007), z(1000000009);
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

		x = BigIntegerSigned<6>(
			BigIntegerSigned<5>(69), BigIntegerUnsigned<5>(42));
		x >>= 3;
		releaseAssert(x == 37044092933);
		x <<= 3;
		releaseAssert(x == 296352743464);

		x = BigIntegerSigned<6>(
			BigIntegerSigned<5>(69), BigIntegerUnsigned<5>(42));
		x /= 5;
		releaseAssert(x == 59270548693);
		x *= 1000000;
		releaseAssert(x == 59270548693000000);
		x /= 1000000009;
		releaseAssert(x == 59270548);
	}

	// Bigger.
	{
		BigIntegerSigned<7> x(18446744073709551557ULL);
		releaseAssert(x % 998244353 == 932051851);
		x /= 998244353;
		releaseAssert(x == 18479187002);
		x = 18446744073709551557ULL;
		x *= 998244353;
		releaseAssert(x.low.low == 1233125317);
		releaseAssert(x.low.high == 4294967282);
		releaseAssert(x.high.low == 998244352);
		releaseAssert(x.high.high == 0);

		using BI = BigIntegerSigned<10>;
		BI a, b, c;
		a = 15;
		BI const d(a), e(20);
		releaseAssert(a == d);
		b = e;
		releaseAssert(b == e);
		c = a + b;
		releaseAssert(c == d + e);
	}

	// Integrations.
	{
		BigIntegerSigned<7> x(18446744073709551557ULL), y(5);
		x = 12 + y + 100 + y + y + 3;
		releaseAssert(x == 130);

		BigIntegerUnsigned<7> const Z(18446744073709551557ULL);
		releaseAssert(leastSignificant1BitIdx(Z - 1u) == 2);
		releaseAssert(bitPopcount(Z) == 60);

		x = 18446744073709551557ULL;
		x *= 1000000009;
		cout << x << endl;
		std::stringstream ss;
		ss << x;
		releaseAssert(
			ss.str() == "18446744239730248220385964013");
		BigIntegerSigned<7> z;
		releaseAssert(z == 0);
		ss >> z;
		cout << z << endl;
		releaseAssert(x == z);
		releaseAssert(z % 18446744073709551557ULL == 0);
		z /= 18446744073709551557ULL;
		cout << z << endl;
		releaseAssert(z == 1000000009);
	}

	return 0;
}

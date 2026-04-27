#include <rain.hpp>

using Rain::Error::releaseAssert;
using namespace Rain::Algorithm;
using namespace Rain;
using namespace std;

int main() {
	using BIFU = BigIntegerFlexUnsigned;

	// Binary.
	{
		BIFU a(5);
		a <<= 3;
		releaseAssert(a == 40);
		a <<= 1;
		releaseAssert(a == 80);
		a >>= 5;
		releaseAssert(a == 2);
		a ^= 24;
		releaseAssert(a == 26);
		a |= 5;
		releaseAssert(a == 31);
		a <<= 62;
		releaseAssert(a.value[1] == 7);
		a <<= 300;
		releaseAssert(a >= ULLONG_MAX);
		a >>= 500;
		releaseAssert(a < 1);

		a = 18446744073709551557ULL;
		for (std::size_t i{6}; i < 64; i++) {
			releaseAssert(static_cast<bool>(a & (BIFU(1) << i)));
		}
		// 111...111100000 (last 1 is at the 2^31 place).
		a = INT_MIN;
		a <<= 4;
		a >>= 33;
		releaseAssert(a == 34359738364);
	}

	// Arithmetic.
	{
		BIFU z{998244353};
		z *= 1000000007;
		cout << z << endl;
		z *= 1000000009;
		cout << z << endl;
		releaseAssert(
			static_cast<std::string>(z) ==
			"998244368971909710889394239");
		z /= 998244353;
		releaseAssert(
			static_cast<std::string>(z) == "1000000016000000063");
		z /= 1000000007;
		releaseAssert(
			static_cast<std::string>(z) == "1000000009");
		z %= 47;
		releaseAssert(z == 44);
		stringstream ss;
		ss << "105483641600945137490864464676928375109110932026"
					"355079724905047142989958503825";
		ss >> z;
		cout << z << endl;
		releaseAssert(
			static_cast<std::string>(z) ==
			"1054836416009451374908644646769283751091109320263550"
			"79724905047142989958503825");
	}
	return 0;
}

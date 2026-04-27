#include <rain.hpp>
#include <random>

using Rain::Error::releaseAssert;
using namespace Rain::Algorithm;
using namespace std;

template<std::size_t LOG_BITS>
using BI = BigIntegerUnsigned<LOG_BITS>;
using BIFU = BigIntegerFlexUnsigned;

template<std::size_t LOG_BITS>
BI<LOG_BITS> generateRandomBI() {
	static std::random_device randomDevice;
	static std::mt19937 generator(randomDevice());

	return BI<LOG_BITS>(generator);
}
template<std::size_t LOG_BITS>
BIFU generateRandomBIFU() {
	static std::random_device randomDevice;
	static std::mt19937 generator(randomDevice());
	static std::uniform_int_distribution<BIFU::T> dist(
		0, ULLONG_MAX);

	BIFU r;
	r.value.resize(1 << (LOG_BITS - 6));
	for (auto &i : r.value) {
		i = dist(generator);
	}
	return r;
}

int main() {
	// BigInt<6>.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		releaseAssert(isPrimeMillerRabin(
			BI<6>(1000000009u), 64, generateRandomBI<6>));
		releaseAssert(isPrimeMillerRabin(
			BI<6>(1000000007u), 64, generateRandomBI<6>));
		releaseAssert(!isPrimeMillerRabin(
			BI<6>(1000000005u), 64, generateRandomBI<6>));
		releaseAssert(isPrimeMillerRabin(
			BI<6>(998244353u), 64, generateRandomBI<6>));
		releaseAssert(!isPrimeMillerRabin(
			BI<6>(998244351u), 64, generateRandomBI<6>));
		releaseAssert(isPrimeMillerRabin(
			BI<6>(29u), 64, generateRandomBI<6>));
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// BigInt<7>.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		releaseAssert(isPrimeMillerRabin(
			BI<7>(8589934583ULL), 64, generateRandomBI<7>));
		releaseAssert(!isPrimeMillerRabin(
			BI<7>(8589934581ULL), 64, generateRandomBI<7>));
		releaseAssert(isPrimeMillerRabin(
			BI<7>(18446744073709551557ULL),
			64,
			generateRandomBI<7>));
		releaseAssert(!isPrimeMillerRabin(
			BI<7>(18446744073709551559ULL),
			64,
			generateRandomBI<7>));
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// Generation.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		static std::random_device randomDevice;
		static std::mt19937 generator(randomDevice());
		BI<7> x;
		cout << "Finding " << 8 * sizeof(x) << "-bit prime..."
				 << endl;
		std::size_t iterations{0};
		do {
			x = decltype(x)(generator);
			iterations++;
			cout << iterations << ": testing " << x << "..."
					 << endl;
		} while (
			isTriviallyComposite(x) ||
			!isPrimeMillerRabin(
				BI<8>(x), 64, generateRandomBI<8>));
		cout << "Found " << x << " in " << iterations
				 << " iterations." << endl;
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// BIFU<7>.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		releaseAssert(isPrimeMillerRabin(
			BIFU(8589934583ULL), 64, generateRandomBIFU<7>));
		releaseAssert(!isPrimeMillerRabin(
			BIFU(8589934581ULL), 64, generateRandomBIFU<7>));
		releaseAssert(isPrimeMillerRabin(
			BIFU(18446744073709551557ULL),
			64,
			generateRandomBIFU<7>));
		releaseAssert(!isPrimeMillerRabin(
			BIFU(18446744073709551559ULL),
			64,
			generateRandomBIFU<7>));
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// BigIntegerFlexUnsigned.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		static std::random_device randomDevice;
		static std::mt19937 generator(randomDevice());
		BIFU x;
		cout << "Finding 128-bit prime..." << endl;
		std::size_t iterations{0};
		do {
			x = generateRandomBIFU<7>();
			iterations++;
			cout << iterations << ": testing " << x << "..."
					 << endl;
		} while (
			isTriviallyComposite(x) ||
			!isPrimeMillerRabin(x, 64, generateRandomBIFU<7>));
		cout << "Found " << x << " in " << iterations
				 << " iterations." << endl;
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}
	return 0;
}

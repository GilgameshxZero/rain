#include <rain.hpp>
#include <random>

using Rain::Error::releaseAssert;
using namespace Rain::Algorithm;
using namespace std;

int main() {
	// BigInt<6>.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		releaseAssert(isPrimeMillerRabin(1000000009u));
		releaseAssert(isPrimeMillerRabin(1000000007u));
		releaseAssert(!isPrimeMillerRabin(1000000005u));
		releaseAssert(isPrimeMillerRabin(998244353u));
		releaseAssert(!isPrimeMillerRabin(998244351u));
		releaseAssert(isPrimeMillerRabin(29u));
		releaseAssert(isPrimeMillerRabin(8589934583u));
		releaseAssert(!isPrimeMillerRabin(8589934581u));
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// BigInt<7>.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		releaseAssert(
			isPrimeMillerRabin(18446744073709551557ULL));
		releaseAssert(
			!isPrimeMillerRabin(18446744073709551559ULL));
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}

	// Generation.
	{
		auto timeBegin{std::chrono::steady_clock::now()};
		static std::random_device randomDevice;
		static std::mt19937 generator(randomDevice());
		BigIntegerUnsigned<7> x;
		cout << "Finding " << 8 * sizeof(x) << "-bit prime..."
				 << endl;
		std::size_t iterations{0};
		do {
			x = decltype(x)(generator);
			iterations++;
			cout << iterations << ": testing " << x << "..."
					 << endl;
		} while (
			isTriviallyComposite(x) || !isPrimeMillerRabin(x));
		cout << "Found " << x << " in " << iterations
				 << " iterations." << endl;
		auto timeElapsed{
			std::chrono::steady_clock::now() - timeBegin};
		cout << "Time: " << timeElapsed << endl;
	}
	return 0;
}

#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;

	// Must use LL or _zu literal to avoid overflow during
	// modding.
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(
			1000000009_zu, 100));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(
			1000000007_zu, 100));
	releaseAssert(!Rain::Algorithm::isPrimeMillerRabin(
		1000000005_zu, 100));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(998244353_zu, 100));
	releaseAssert(!Rain::Algorithm::isPrimeMillerRabin(
		998244351_zu, 100));

	// To test up to 64 bits, need to use 128-bit integer.
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(
			Rain::Algorithm::BigIntegerUnsigned<7>(29), 100));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(
			Rain::Algorithm::BigIntegerUnsigned<7>(8589934583),
			100));
	releaseAssert(!Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntegerUnsigned<7>(8589934581),
		100));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabin(
			Rain::Algorithm::BigIntegerUnsigned<7>(
				18446744073709551557ULL),
			100));
	releaseAssert(!Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntegerUnsigned<7>(
			18446744073709551559ULL),
		100));

	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabinDeterministic(
			1000000009_zu));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabinDeterministic(
			1000000007_zu));
	releaseAssert(
		!Rain::Algorithm::isPrimeMillerRabinDeterministic(
			1000000005_zu));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabinDeterministic(
			998244353_zu));
	releaseAssert(
		!Rain::Algorithm::isPrimeMillerRabinDeterministic(
			998244351_zu));
	releaseAssert(
		Rain::Algorithm::isPrimeMillerRabinDeterministic(
			Rain::Algorithm::BigIntegerUnsigned<8>(
				18446744073709551557ULL)));
	return 0;
}

#include <rain.hpp>

int main() {
	using namespace Rain::Literal;

	// Must use LL or _zu literal to avoid overflow during modding.
	assert(Rain::Algorithm::isPrimeMillerRabin(1000000009_zu, 100));
	assert(Rain::Algorithm::isPrimeMillerRabin(1000000007_zu, 100));
	assert(!Rain::Algorithm::isPrimeMillerRabin(1000000005_zu, 100));
	assert(Rain::Algorithm::isPrimeMillerRabin(998244353_zu, 100));
	assert(!Rain::Algorithm::isPrimeMillerRabin(998244351_zu, 100));

	// To test up to 64 bits, need to use 128-bit integer.
	assert(Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntSigned<7>(29), 100));
	assert(Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntSigned<7>(8589934583), 100));
	assert(!Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntSigned<7>(8589934581), 100));
	assert(Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntUnsigned<7>(18446744073709551557ULL), 100));
	assert(!Rain::Algorithm::isPrimeMillerRabin(
		Rain::Algorithm::BigIntUnsigned<7>(18446744073709551559ULL), 100));

	assert(Rain::Algorithm::isPrimeMillerRabinDeterministic(1000000009_zu));
	assert(Rain::Algorithm::isPrimeMillerRabinDeterministic(1000000007_zu));
	assert(!Rain::Algorithm::isPrimeMillerRabinDeterministic(1000000005_zu));
	assert(Rain::Algorithm::isPrimeMillerRabinDeterministic(998244353_zu));
	assert(!Rain::Algorithm::isPrimeMillerRabinDeterministic(998244351_zu));
	assert(Rain::Algorithm::isPrimeMillerRabinDeterministic(
		Rain::Algorithm::BigIntSigned<8>(18446744073709551557ULL)));
	return 0;
}

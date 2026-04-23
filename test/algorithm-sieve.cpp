#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	auto [minFactor, primes]{
		Rain::Algorithm::linearSieve(1000000)};
	releaseAssert(primes.size() == 78498);
	releaseAssert(minFactor[799] == 6);
	releaseAssert(799 % primes[minFactor[799]] == 0);
	return 0;
}

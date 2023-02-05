#include <rain/algorithm/sieve.hpp>

#include <cassert>

int main() {
	auto [minFactor, primes]{Rain::Algorithm::linearSieve(1000000)};
	assert(primes.size() == 78498);
	assert(minFactor[799] == 17);
	return 0;
}

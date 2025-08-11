#include <rain.hpp>

int main() {
	auto [minFactor, primes]{Rain::Algorithm::linearSieve(1000000)};
	assert(primes.size() == 78498);
	assert(minFactor[799] == 6);
	assert(799 % primes[minFactor[799]] == 0);
	return 0;
}

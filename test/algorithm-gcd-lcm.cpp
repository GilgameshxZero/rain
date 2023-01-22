#include <rain/algorithm/gcd-lcm.hpp>

#include <cassert>

int main() {
	assert(Rain::Algorithm::greatestCommonDivisor(948, 720) == 12);
	assert(Rain::Algorithm::leastCommonMultiple(948, 720) == 56880);
	return 0;
}

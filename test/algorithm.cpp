// General algorithm function tests.
#include <rain/algorithm/algorithm.hpp>

#include <cassert>

int main() {
	assert(Rain::Algorithm::mostSignificant1BitIdx(0x080c3948) == 27);
	assert(Rain::Algorithm::leastSignificant1BitIdx(0x080c3948) == 3);
	assert(Rain::Algorithm::bitPopcount(0xf80c3948) == 13);
	assert(Rain::Algorithm::greatestCommonDivisor(948, 720) == 12);
	assert(Rain::Algorithm::leastCommonMultiple(948, 720) == 56880);
	return 0;
}

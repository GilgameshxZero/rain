#include <rain/algorithm/bit-manipulators.hpp>

#include <cassert>

int main() {
	assert(Rain::Algorithm::mostSignificant1BitIdx(0x080c3948) == 27);
	assert(Rain::Algorithm::leastSignificant1BitIdx(0x080c3948) == 3);
	assert(Rain::Algorithm::bitPopcount(0xf80c3948) == 13);
	return 0;
}

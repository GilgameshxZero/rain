#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	releaseAssert(
		Rain::Algorithm::mostSignificant1BitIdx(0x080c3948u) ==
		27);
	releaseAssert(
		Rain::Algorithm::leastSignificant1BitIdx(0x080c3948u) ==
		3);
	releaseAssert(
		Rain::Algorithm::bitPopcount(0xf80c3948) == 13);
	return 0;
}

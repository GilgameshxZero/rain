#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	releaseAssert(
		Rain::Algorithm::greatestCommonDivisor(948, 720) == 12);
	auto [d, cX, cY]{
		Rain::Algorithm::greatestCommonDivisorExtended(
			948, 720)};
	releaseAssert(d == 12);
	releaseAssert(cX * 948 + cY * 720 == d);
	releaseAssert(
		Rain::Algorithm::leastCommonMultiple(948, 720) ==
		56880);
	return 0;
}

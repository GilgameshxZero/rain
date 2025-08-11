#include <rain.hpp>

int main() {
	assert(Rain::Algorithm::greatestCommonDivisor(948, 720) == 12);
	auto [d, cX, cY]{Rain::Algorithm::greatestCommonDivisorExtended(948, 720)};
	assert(d == 12);
	assert(cX * 948 + cY * 720 == d);
	assert(Rain::Algorithm::leastCommonMultiple(948, 720) == 56880);
	return 0;
}

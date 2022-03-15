// General algorithm function tests.
#include <rain/algorithm/algorithm.hpp>

#include <cassert>
#include <iostream>

int main() {
	assert(Rain::Algorithm::mostSignificant1BitIdx(0x080c3948) == 27);
	assert(Rain::Algorithm::leastSignificant1BitIdx(0x080c3948) == 3);
	assert(Rain::Algorithm::bitPopcount(0xf80c3948) == 13);
	assert(Rain::Algorithm::greatestCommonDivisor(948, 720) == 12);
	assert(Rain::Algorithm::leastCommonMultiple(948, 720) == 56880);

	assert(Rain::Algorithm::fibonacciNumber(0) == 0);
	assert(Rain::Algorithm::fibonacciNumber(1) == 1);
	assert(Rain::Algorithm::fibonacciNumber(2) == 1);
	assert(Rain::Algorithm::fibonacciNumber(3) == 2);
	assert(Rain::Algorithm::fibonacciNumber(4) == 3);
	assert(Rain::Algorithm::fibonacciNumber(5) == 5);
	assert(Rain::Algorithm::fibonacciNumber(6) == 8);
	assert(Rain::Algorithm::fibonacciNumber(7) == 13);
	assert(Rain::Algorithm::fibonacciNumber(8) == 21);
	assert(Rain::Algorithm::fibonacciNumber(9) == 34);
	assert(Rain::Algorithm::fibonacciNumber(10) == 55);
	return 0;
}

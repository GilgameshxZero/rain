#include <rain.hpp>

using namespace Rain;
using namespace Math;
using namespace Error;

int main() {
	releaseAssert(fibonacciNumber(0) == 0);
	releaseAssert(fibonacciNumber(1) == 1);
	releaseAssert(fibonacciNumber(2) == 1);
	releaseAssert(fibonacciNumber(3) == 2);
	releaseAssert(fibonacciNumber(4) == 3);
	releaseAssert(fibonacciNumber(5) == 5);
	releaseAssert(fibonacciNumber(6) == 8);
	releaseAssert(fibonacciNumber(7) == 13);
	releaseAssert(fibonacciNumber(8) == 21);
	releaseAssert(fibonacciNumber(9) == 34);
	releaseAssert(fibonacciNumber(10) == 55);
	return 0;
}

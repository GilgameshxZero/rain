#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	releaseAssert(Rain::Algorithm::fibonacciNumber(0) == 0);
	releaseAssert(Rain::Algorithm::fibonacciNumber(1) == 1);
	releaseAssert(Rain::Algorithm::fibonacciNumber(2) == 1);
	releaseAssert(Rain::Algorithm::fibonacciNumber(3) == 2);
	releaseAssert(Rain::Algorithm::fibonacciNumber(4) == 3);
	releaseAssert(Rain::Algorithm::fibonacciNumber(5) == 5);
	releaseAssert(Rain::Algorithm::fibonacciNumber(6) == 8);
	releaseAssert(Rain::Algorithm::fibonacciNumber(7) == 13);
	releaseAssert(Rain::Algorithm::fibonacciNumber(8) == 21);
	releaseAssert(Rain::Algorithm::fibonacciNumber(9) == 34);
	releaseAssert(Rain::Algorithm::fibonacciNumber(10) == 55);
	return 0;
}

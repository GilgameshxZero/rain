#include <rain/algorithm/fibonacci.hpp>

#include <cassert>

int main() {
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

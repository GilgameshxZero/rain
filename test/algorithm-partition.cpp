#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	auto partitions{Rain::Algorithm::partitionNumbers(300LL)};
	releaseAssert(partitions[300] == 9253082936723602);
	return 0;
}

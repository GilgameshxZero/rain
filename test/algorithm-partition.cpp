#include <rain.hpp>

int main() {
	auto partitions{Rain::Algorithm::partitionNumbers(300LL)};
	assert(partitions[300] == 9253082936723602);
	return 0;
}

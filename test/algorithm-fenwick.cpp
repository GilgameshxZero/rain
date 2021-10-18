// Tests for Fenwick tree.
#include <rain/algorithm/fenwick.hpp>

#include <cassert>

int main() {
	Rain::Algorithm::FenwickTree fenwick;
	fenwick.delta(0, 100);
	fenwick.delta(1, 5);
	fenwick.delta(2, 10);
	assert(fenwick.sum(2) == 115);
	fenwick.delta(1, -26);
	assert(fenwick.sum(9) == 89);
	// 100, -21, 10.

	// 100, -21, 10, 0, 0, 0, 0, 0, 0, 7.
	fenwick.delta(9, 7);
	assert(fenwick.sum(8) == 89);
	// 100, -21, 15, 0, 0, 0, 0, 0, 0, 7.
	fenwick.delta(2, 5);
	// 100, -21, 15, 0, 0, 8, 0, 0, 0, 7.
	fenwick.delta(5, 8);
	// 100, -21, 15, 0, -90, 8, 0, 0, 0, 7.
	fenwick.delta(4, -90);
	assert(fenwick.sum(5) - fenwick.sum(3) == -82);
	assert(fenwick.sum(9) - fenwick.sum(5) == 7);
	assert(fenwick.sum(7) == 12);
	assert(fenwick.sum(100000) == 19);

	return 0;
}

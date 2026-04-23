// Tests for Fenwick tree.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	Rain::Algorithm::FenwickTree fenwick(100001);
	fenwick.modify(0, 100);
	fenwick.modify(1, 5);
	fenwick.modify(2, 10);
	releaseAssert(fenwick.sum(2) == 115);
	fenwick.modify(1, -26);
	releaseAssert(fenwick.sum(9) == 89);
	// 100, -21, 10.

	fenwick.modify(9, 7);
	// 100, -21, 10, 0, 0, 0, 0, 0, 0, 7.
	releaseAssert(fenwick.sum(8) == 89);
	fenwick.modify(2, 5);
	// 100, -21, 15, 0, 0, 0, 0, 0, 0, 7.
	fenwick.modify(5, 8);
	// 100, -21, 15, 0, 0, 8, 0, 0, 0, 7.
	fenwick.modify(4, -90);
	// 100, -21, 15, 0, -90, 8, 0, 0, 0, 7.
	releaseAssert(fenwick.sum(5) - fenwick.sum(3) == -82);
	releaseAssert(fenwick.sum(9) - fenwick.sum(5) == 7);
	releaseAssert(fenwick.sum(7) == 12);
	releaseAssert(fenwick.sum(100000) == 19);

	return 0;
}

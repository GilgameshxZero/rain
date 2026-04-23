// Tests for DSU/union-find.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	Rain::Algorithm::DisjointSetUnion dsu(10);
	releaseAssert(dsu.find(0) == 0);
	releaseAssert(dsu.find(3) == 3);
	dsu.join(0, 1);
	dsu.join(1, 2);
	releaseAssert(dsu.find(0) == dsu.find(2));
	releaseAssert(dsu.rank(0) == 3);
	releaseAssert(dsu.rank(4) == 1);
	releaseAssert(dsu.rank(2) == 3);
	releaseAssert(dsu.rank(1) == 3);
	dsu.join(9, 8);
	releaseAssert(dsu.join(1, 9));
	releaseAssert(dsu.rank(1) == 5);
	releaseAssert(dsu.find(9) == dsu.find(0));
	releaseAssert(!dsu.join(2, 8));
	return 0;
}

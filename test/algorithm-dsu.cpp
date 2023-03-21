// Tests for DSU/union-find.
#include <rain/algorithm/dsu.hpp>

#include <cassert>

int main() {
	Rain::Algorithm::DisjointSetUnion dsu(10);
	assert(dsu.find(0) == 0);
	assert(dsu.find(3) == 3);
	dsu.join(0, 1);
	dsu.join(1, 2);
	assert(dsu.find(0) == dsu.find(2));
	assert(dsu.rank(0) == 3);
	assert(dsu.rank(4) == 1);
	assert(dsu.rank(2) == 3);
	assert(dsu.rank(1) == 3);
	dsu.join(9, 8);
	assert(dsu.join(1, 9));
	assert(dsu.rank(1) == 5);
	assert(dsu.find(9) == dsu.find(0));
	assert(!dsu.join(2, 8));
	return 0;
}

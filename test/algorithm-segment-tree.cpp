// Tests for segment tree.
#include <rain/algorithm/segment-tree.hpp>

#include <cassert>

// Underlying array of LLs, with sum segments.
class SumTree : public Rain::Algorithm::SegmentTree<long long, long long> {
	using Rain::Algorithm::SegmentTree<long long, long long>::SegmentTree;

	protected:
	virtual long long aggregate(
		long long const &left,
		long long const &right,
		std::pair<std::size_t, std::size_t> const &,
		std::size_t const) override {
		return left + right;
	}
	virtual void propagate(
		long long const &update,
		long long &left,
		long long &right,
		std::pair<std::size_t, std::size_t> const &,
		std::size_t const) override {
		left += update;
		right += update;
	}
	virtual void apply(
		long long &value,
		long long const &update,
		std::pair<std::size_t, std::size_t> const &range,
		std::size_t const) override {
		value += update * (range.second - range.first + 1);
	}
};

int main() {
	/*
											0
					 |                     \
					 1                      2
		 |            \            |     \
		 3             4           5      6
	|      \      |     \      |   \   | \
	7       8     9     10    11   12 13 14
	|  \   | \   | \   | \   | \   |
	15 16 17 18 19 20 21 22 23 24 25
	*/
	SumTree sumTree(11);
	// [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0].
	sumTree.update(0, 5, 7);
	// [7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0].
	assert(sumTree.query(1, 3) == 21);
	assert(sumTree.query(0, 10) == 42);
	assert(sumTree.query(1, 1) == 7);
	assert(sumTree.query(5, 9) == 7);
	sumTree.update(5, 9, -5);
	// [7, 7, 7, 7, 7, 2, -5, -5, -5, -5, 0].
	assert(sumTree.query(5, 5) == 2);
	assert(sumTree.query(4, 6) == 4);
	assert(sumTree.query(0, 10) == 17);
	sumTree.update(0, 10, -1);
	// [6, 6, 6, 6, 6, 1, -6, -6, -6, -6, -1].
	sumTree.update(1, 3, 13);
	// [6, 19, 19, 19, 6, 1, -6, -6, -6, -6, -1].
	sumTree.update(3, 6, -10);
	// [6, 19, 19, 9, -4, -9, -16, -6, -6, -6, -1].
	assert(sumTree.query(0, 7) == 18);
	assert(sumTree.query(8, 10) == -13);
	assert(sumTree.query(0, 10) == 5);
	sumTree.update(0, 3, -5);
	// [1, 14, 14, 4, -4, -9, -16, -6, -6, -6, -1].
	sumTree.update(4, 7, 3);
	// [1, 14, 14, 4, -1, -6, -13, -3, -6, -6, -1].
	assert(sumTree.query(0, 7) == 10);

	return 0;
}

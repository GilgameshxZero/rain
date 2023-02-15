// Tests for segment tree.
#include <rain/algorithm/algorithm.hpp>
#include <rain/algorithm/segment-tree.hpp>
#include <rain/literal.hpp>
#include <rain/time.hpp>

#include <cassert>
#include <iostream>

namespace MaxTreeAux {
	void aggregateValues(
		typename std::vector<long long>::reference value,
		long long const &left,
		long long const &right) {
		value = max(left, right);
	}
	long long aggregateResults(long long const &left, long long const &right) {
		return max(left, right);
	}
	void apply(
		typename std::vector<long long>::reference value,
		long long const &update) {
		value = max(value, update);
	}
}
using MaxTree = Rain::Algorithm::SegmentTree<
	long long,
	long long,
	long long,
	-1,
	MaxTreeAux::aggregateValues,
	MaxTreeAux::aggregateResults,
	MaxTreeAux::apply>;

namespace SumTreeAux {
	void aggregateValues(
		typename std::vector<long long>::reference value,
		long long const &left,
		long long const &right) {
		value = left + right;
	}
	long long aggregateResults(long long const &left, long long const &right) {
		return left + right;
	}
	void apply(
		typename std::vector<long long>::reference value,
		long long const &update) {
		value += update;
	}
}
using SumTree = Rain::Algorithm::SegmentTree<
	long long,
	long long,
	long long,
	0,
	SumTreeAux::aggregateValues,
	SumTreeAux::aggregateResults,
	SumTreeAux::apply>;

int main() {
	using namespace Rain::Literal;

	{
		MaxTree tree(10);
		assert(tree.query(0, 9) == -1);
		assert(tree.query(1, 1) == -1);
		assert(tree.query(5, 5) == -1);
		tree.update(5, 0);
		assert(tree.query(5, 5) == 0);
		assert(tree.query(4, 6) == 0);
		assert(tree.query(1, 1) == -1);
	}

	{
		// Same as Fenwick tree tests.
		SumTree tree(100001);
		tree.update(0, 100);
		tree.update(1, 5);
		tree.update(2, 10);
		assert(tree.query(0, 2) == 115);
		tree.update(1, -26);
		assert(tree.query(0, 9) == 89);
		// 100, -21, 10.

		tree.update(9, 7);
		// 100, -21, 10, 0, 0, 0, 0, 0, 0, 7.
		assert(tree.query(0, 8) == 89);
		tree.update(2, 5);
		// 100, -21, 15, 0, 0, 0, 0, 0, 0, 7.
		tree.update(5, 8);
		// 100, -21, 15, 0, 0, 8, 0, 0, 0, 7.
		tree.update(4, -90);
		// 100, -21, 15, 0, -90, 8, 0, 0, 0, 7.
		assert(tree.query(4, 5) == -82);
		assert(tree.query(6, 9) == 7);
		assert(tree.query(0, 7) == 12);
		assert(tree.query(0, 100000) == 19);
	}

	return 0;
}

// Tests for segment tree.
#include <rain/algorithm/algorithm.hpp>
#include <rain/algorithm/segment-tree.hpp>
#include <rain/literal.hpp>
#include <rain/time.hpp>

#include <cassert>
#include <iostream>

class MaxTreePolicy {
	public:
	using Value = long long;
	using Result = long long;
	using Update = long long;

	static constexpr Value DEFAULT_VALUE{-1};
	inline static void apply(Value &value, Update const &update) {
		value = max(value, update);
	}
	inline static Result aggregate(Result const &left, Result const &right) {
		return max(left, right);
	}
	inline static void
	retrace(Value &value, Value const &left, Value const &right) {
		value = max(left, right);
	}
};
using MaxTree = Rain::Algorithm::SegmentTree<MaxTreePolicy>;

class SumTreePolicy {
	public:
	using Value = long long;
	using Result = long long;
	using Update = long long;

	static constexpr Value DEFAULT_VALUE{0};
	inline static void apply(Value &value, Update const &update) {
		value += update;
	}
	inline static Result aggregate(Result const &left, Result const &right) {
		return left + right;
	}
	inline static void
	retrace(Value &value, Value const &left, Value const &right) {
		value = left + right;
	}
};
using SumTree = Rain::Algorithm::SegmentTree<SumTreePolicy>;

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

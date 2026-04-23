// Tests for segment tree.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Algorithm;

	{
		SegmentTreeLazy<SegmentTreeLazy<>::PolicySum<int>>
			sumTree(4);
		sumTree.update(0, 3, 1);
		sumTree.update(1, 2, 5);
		sumTree.update(2, 2, -20);
		releaseAssert(sumTree.query(0, 3) == -6);
		releaseAssert(sumTree.query(0, 2) == -7);
		releaseAssert(sumTree.query(0, 1) == 7);

		SegmentTreeLazy<SegmentTreeLazy<>::PolicyMin<int>>
			minTree(4);
		minTree.update(0, 3, 1);
		minTree.update(1, 2, 5);
		minTree.update(2, 2, -20);
		releaseAssert(minTree.query(0, 3) == -14);
		releaseAssert(minTree.query(0, 2) == -14);
		releaseAssert(minTree.query(0, 1) == 1);

		SegmentTreeLazy<SegmentTreeLazy<>::PolicyMax<int>>
			maxTree(4);
		maxTree.update(0, 3, 1);
		maxTree.update(1, 2, 5);
		maxTree.update(2, 2, -20);
		releaseAssert(maxTree.query(0, 3) == 6);
		releaseAssert(maxTree.query(0, 2) == 6);
		releaseAssert(maxTree.query(0, 1) == 6);
	}

	{
		/*
												1
						|                     \
						2                      3
			|            \            |     \
			4             5           6      7
		|      \      |     \      |   \   | \
		8       9     10     11    12   13 14 15
		|  \    | \   | \
		16 17   18 19 20 21
		*/
		SegmentTreeLazy<SegmentTreeLazy<>::PolicySum<int>>
			sumTree(11);
		// [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0].
		sumTree.update(0, 5, 7);
		// [7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0].
		releaseAssert(sumTree.query(1, 3) == 21);
		releaseAssert(sumTree.query(0, 10) == 42);
		releaseAssert(sumTree.query(1, 1) == 7);
		releaseAssert(sumTree.query(5, 9) == 7);
		sumTree.update(5, 9, -5);
		// [7, 7, 7, 7, 7, 2, -5, -5, -5, -5, 0].
		releaseAssert(sumTree.query(5, 5) == 2);
		releaseAssert(sumTree.query(4, 6) == 4);
		releaseAssert(sumTree.query(0, 10) == 17);
		sumTree.update(0, 10, -1);
		// [6, 6, 6, 6, 6, 1, -6, -6, -6, -6, -1].
		sumTree.update(1, 3, 13);
		// [6, 19, 19, 19, 6, 1, -6, -6, -6, -6, -1].
		sumTree.update(3, 6, -10);
		// [6, 19, 19, 9, -4, -9, -16, -6, -6, -6, -1].
		releaseAssert(sumTree.query(0, 7) == 18);
		releaseAssert(sumTree.query(8, 10) == -13);
		releaseAssert(sumTree.query(0, 10) == 5);
		sumTree.update(0, 3, -5);
		// [1, 14, 14, 4, -4, -9, -16, -6, -6, -6, -1].
		sumTree.update(4, 7, 3);
		// [1, 14, 14, 4, -1, -6, -13, -3, -6, -6, -1].
		releaseAssert(sumTree.query(0, 7) == 10);

		std::vector<int> underlying{
			{1, 14, 14, 4, -1, -6, -13, -3, -6, -6, -1}};
		auto it{sumTree.frontUnderlying()};
		for (std::size_t i{0}; i < underlying.size(); i++) {
			releaseAssert(underlying[i] == *it);
			if (!it.isBackUnderlying()) {
				it = it.nextUnderlying();
			}
		}
		it = sumTree.root();
		releaseAssert(*it == -3);
		it = it.left();
		releaseAssert(*it == 1 + -6 + -13 + -3 + -6 + -6 + -1);
		it = it.right();
		releaseAssert(*it == 1 + -6 + -1);
		it = it.left();
		releaseAssert(!it.isLeaf());
		releaseAssert(*it == -6 + -1);
		it = it.right();
		releaseAssert(it.isLeaf());
		releaseAssert(it.isBackUnderlying());
		releaseAssert(*it == -1);

		sumTree.update(10, 10, 1);
		// [1, 14, 14, 4, -1, -6, -13, -3, -6, -6, 0].
		sumTree.update(9, 9, 1);
		// [1, 14, 14, 4, -1, -6, -13, -3, -6, -5, 0].
	}

	{
		// Time test.
		auto timeBegin = std::chrono::steady_clock::now();
		SegmentTreeLazy<SegmentTreeLazy<>::PolicySum<long long>>
			tree2(100000);
		for (std::size_t i{0}; i < 100000; i++) {
			std::pair<std::size_t, std::size_t> range{
				rand() % 100000, rand() % 100000};
			if (range.first > range.second) {
				std::swap(range.first, range.second);
			}
			tree2.update(range.first, range.second, rand());
		}
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << ".\n";
	}

	{
		// Assume this is a chess board.
		SegmentTreeLazy<
			SegmentTreeLazy<>::PolicySum2DPoint<int, 8>>
			tree(8);
		// 6B +13.
		tree.update(2, 2, {1, 13});
		// Query entire board.
		releaseAssert(tree.query(0, 7, 7) == 13);
		// Just the top-left 2x2.
		releaseAssert(tree.query(0, 1, 1) == 0);
		releaseAssert(tree.query(0, 2, 1) == 13);
		// Place point values of white pieces.
		for (size_t i{0}; i < 8; i++) {
			tree.update(6, 6, {i, 1});
		}
		tree.update(7, 7, {0, 5});
		tree.update(7, 7, {1, 3});
		tree.update(7, 7, {2, 3});
		tree.update(7, 7, {3, 9});
		tree.update(7, 7, {4, 100});
		tree.update(7, 7, {5, 3});
		tree.update(7, 7, {6, 3});
		tree.update(7, 7, {7, 5});
		// All pawns.
		releaseAssert(
			tree.query(0, 6, 7) - tree.query(0, 5, 7) == 8);
		// All non-pawn white pieces.
		releaseAssert(
			tree.query(0, 7, 7) - tree.query(0, 6, 7) == 131);
		// Entire board.
		releaseAssert(tree.query(0, 7, 7) == 13 + 8 + 131);
	}

	return 0;
}

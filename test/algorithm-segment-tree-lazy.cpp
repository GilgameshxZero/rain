// Tests for segment tree.
#include <rain/algorithm/segment-tree-lazy.hpp>
#include <rain/literal.hpp>
#include <rain/time.hpp>

#include <cassert>
#include <iostream>

namespace SumTreeAux {
	void aggregateValues(
		std::size_t const,
		typename std::vector<long long>::reference value,
		long long const &left,
		long long const &right,
		std::pair<std::size_t, std::size_t> const &) {
		value = left + right;
	}
	long long aggregateResults(
		std::size_t const,
		long long const &left,
		long long const &right,
		std::pair<std::size_t, std::size_t> const &) {
		return left + right;
	}
	void split(
		std::size_t const,
		long long const &update,
		typename std::vector<long long>::reference left,
		typename std::vector<long long>::reference right,
		std::pair<std::size_t, std::size_t> const &) {
		left += update;
		right += update;
	}
	void apply(
		std::size_t const,
		typename std::vector<long long>::reference value,
		long long const &update,
		std::pair<std::size_t, std::size_t> const &range) {
		value += update * (range.second - range.first + 1);
	}
	long long convert(std::size_t const, long long const &value) {
		return value;
	}
}
using SumTree = Rain::Algorithm::SegmentTreeLazy<
	long long,
	long long,
	long long,
	0,
	SumTreeAux::aggregateValues,
	SumTreeAux::aggregateResults,
	SumTreeAux::split,
	SumTreeAux::apply,
	SumTreeAux::convert>;

int main() {
	using namespace Rain::Literal;

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

	// Time test.
	auto timeBegin = std::chrono::steady_clock::now();
	SumTree tree2(100000);
	for (std::size_t i{0}; i < 100000; i++) {
		std::pair<std::size_t, std::size_t> range{rand() % 100000, rand() % 100000};
		if (range.first > range.second) {
			std::swap(range.first, range.second);
		}
		tree2.update(range.first, range.second, rand());
	}
	auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
	std::cout << "Time elapsed: " << timeElapsed << ".\n";

	return 0;
}
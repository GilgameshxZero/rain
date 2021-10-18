// Fenwick/Binary-Indexed Tree implementation. O(ln N) point updates and range
// queries. Not thread-safe.
#pragma once

#include <vector>

namespace Rain::Algorithm {
	// Fixed-size Fenwick/Binary-Indexed Tree implementation. O(ln N) point
	// updates and range queries. Not thread-safe.
	//
	// Value must support adding, and should be lightly copyable. In addition,
	// default initialization should be equivalent to "0".
	template <typename Value = long long>
	class FenwickTree {
		private:
		std::vector<Value> tree;

		public:
		// Creates a Fenwick tree, which may be resized by operations.
		FenwickTree(std::size_t const size = 0) : tree(size) {}

		// Computes prefix sum up to and including idx.
		Value sum(std::size_t const idx) {
			this->reserve(idx + 1);
			Value aggregate{};
			for (std::size_t i = idx; i != SIZE_MAX; i &= i + 1, i--) {
				aggregate += this->tree[i];
			}
			return aggregate;
		}

		// Modify index by a delta.
		void delta(std::size_t const idx, Value const &delta) {
			this->reserve(idx + 1);
			for (std::size_t i = idx; i < this->tree.size(); i |= i + 1) {
				this->tree[i] += delta;
			}
		}

		private:
		// Expand the Fenwick tree in O(delta * ln N) with default-initialized
		// Values.
		void reserve(std::size_t const size) {
			while (this->tree.size() < size) {
				this->tree.push_back({});

				// Sum up the segment for i.
				std::size_t i = this->tree.size() - 1, k = (i & (i + 1)) - 1;
				for (std::size_t j = i - 1; j != k; j &= j + 1, j--) {
					this->tree.back() += this->tree[j];
				}
			}
		}
	};
}

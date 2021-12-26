// Union-Find/Disjoint-Set-Union implementation. Near constant time amortized
// union and find.
#pragma once

#include "../literal.hpp"

#include <vector>

namespace Rain::Algorithm {
	// Union-Find/Disjoint-Set-Union implementation. Near-constant time amortized
	// union and find.
	//
	// Implements path compression and union by rank.
	class DisjointSetUnion {
		private:
		// A pair of (is_root, X). If node is root, X stores the size of the
		// cluster. Otherwise, X stores the index of the node’s parent.
		mutable std::vector<std::pair<bool, std::size_t>> nodes;

		public:
		DisjointSetUnion(std::size_t const size) : nodes(size, {true, 1}) {}

		std::size_t find(std::size_t const i) const {
			if (this->nodes[i].first) {
				return i;
			}
			return this->nodes[i].second = this->find(this->nodes[i].second);
		}
		std::size_t rank(std::size_t const i) const {
			return this->nodes[this->find(i)].second;
		}
		void join(std::size_t const i, std::size_t const j) {
			std::size_t pI = this->find(i), pJ = this->find(j);
			if (pI == pJ) {
				return;
			}
			if (this->nodes[pI].second > this->nodes[pJ].second) {
				std::swap(pI, pJ);
			}
			this->nodes[pJ].second += this->nodes[pI].second;
			this->nodes[pI] = {false, pJ};
		}
	};
}

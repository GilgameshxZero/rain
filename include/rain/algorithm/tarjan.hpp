// Tarjan’s strongly connected components algorithm.
#pragma once

#include <limits>
#include <stack>
#include <unordered_set>
#include <vector>

namespace Rain::Algorithm {
	// Computes strongly connected components (SCCs) for a simple graph G in
	// O(V+E). The SCCs form an acyclic condensation graph of G. Typically,
	// Tarjan's is more efficient than Kosarju's algorithm for SCCs, though
	// Kosarju's algorithm provides the SCCs in topologically sorted order (and
	// Tarjan's in reverse topological order).
	//
	// Returns the number of CCs, and the 0-indexed index of the
	// SCC that each vertex belongs to.
	inline std::pair<std::size_t, std::vector<std::size_t>> sccTarjan(
		std::vector<std::unordered_set<std::size_t>> const &edges) {
		std::size_t cScc{0}, cPreOrderId{0};
		std::vector<std::size_t> scc(edges.size()),
			preOrderId(edges.size(), std::numeric_limits<std::size_t>::max()),
			lowLink(edges.size());
		std::vector<bool> onStack(edges.size(), false);
		std::stack<std::size_t> s;

		// Tarjan’s requires a subroutine for its inner DFS, which we implement via
		// recursive lambda with an explicit return type.
		auto subroutine{[&](std::size_t _i) {
			auto subroutineInner{[&](std::size_t i, auto &subroutineRef) -> void {
				lowLink[i] = preOrderId[i] = cPreOrderId++;
				onStack[i] = true;
				s.push(i);

				for (auto const &j : edges[i]) {
					if (preOrderId[j] == std::numeric_limits<std::size_t>::max()) {
						subroutineRef(j, subroutineRef);
						lowLink[i] = std::min(lowLink[i], lowLink[j]);
					} else if (onStack[j]) {
						lowLink[i] = std::min(lowLink[i], preOrderId[j]);
					}
				}

				if (lowLink[i] == preOrderId[i]) {
					std::size_t j;
					do {
						j = s.top();
						s.pop();
						onStack[j] = false;
						scc[j] = cScc;
					} while (j != i);
					cScc++;
				}
			}};
			subroutineInner(_i, subroutineInner);
		}};

		for (std::size_t i{0}; i < edges.size(); i++) {
			if (preOrderId[i] == std::numeric_limits<std::size_t>::max()) {
				subroutine(i);
			}
		}

		// C++17: guaranteed either NRVO or move.
		return {cScc, scc};
	}
}

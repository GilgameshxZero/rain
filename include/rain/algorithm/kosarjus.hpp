// Kosarju strongly connected components algorithm.
#pragma once

#include <unordered_set>
#include <vector>

namespace Rain::Algorithm {
	// Computes strongly connected components (SCCs) for a simple graph G in
	// O(V+E). The SCCs form an acyclic condensation graph of G. Kosarju’s
	// algorithm provides the SCCs in topologically sorted order, but is a
	// constant factor slower than Tarjan’s.
	//
	// Returns the number of CCs, and the 0-indexed index of the
	// SCC that each vertex belongs to. SCC indices are sorted in topological
	// order (lower indices have edges pointing toward higher indices).
	std::pair<std::size_t, std::vector<std::size_t>> stronglyConnectedKosarjus(
		std::vector<std::unordered_set<std::size_t>> const &edges) {
		std::size_t cScc{0}, cPostOrderId{0};
		std::vector<std::size_t> scc(edges.size(), SIZE_MAX),
			postOrderId(edges.size(), SIZE_MAX);

		auto subroutineFirst{[&](std::size_t _i) {
			auto subroutineInner{[&](std::size_t i, auto &subroutineRef) -> void {
				// Temporarily mark as visited.
				postOrderId[i] = 0;

				for (auto const &j : edges[i]) {
					if (postOrderId[j] == SIZE_MAX) {
						subroutineRef(j, subroutineRef);
					}
				}
				postOrderId[i] = cPostOrderId++;
			}};
			subroutineInner(_i, subroutineInner);
		}};

		std::vector<std::size_t> vPostOrder(edges.size());
		std::vector<std::unordered_set<std::size_t>> transposeEdges(edges.size());
		for (std::size_t i{0}; i < edges.size(); i++) {
			if (postOrderId[i] == SIZE_MAX) {
				subroutineFirst(i);
			}
			vPostOrder[postOrderId[i]] = i;

			// Second part operates on transpose of graph.
			for (auto const &j : edges[i]) {
				transposeEdges[j].insert(i);
			}
		}

		auto subroutineSecond{[&](std::size_t _i) {
			auto subroutineInner{[&](std::size_t i, auto &subroutineRef) -> void {
				scc[i] = cScc;
				for (auto const &j : transposeEdges[i]) {
					if (scc[j] == SIZE_MAX) {
						subroutineRef(j, subroutineRef);
					}
				}
			}};
			subroutineInner(_i, subroutineInner);
		}};

		for (std::size_t i{0}; i < vPostOrder.size(); i++) {
			if (scc[vPostOrder[vPostOrder.size() - 1 - i]] == SIZE_MAX) {
				subroutineSecond(vPostOrder[vPostOrder.size() - 1 - i]);
				cScc++;
			}
		}

		return {cScc, scc};
	}
}

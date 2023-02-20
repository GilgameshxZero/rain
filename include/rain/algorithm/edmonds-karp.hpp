// Edmonds-Karp max-flow.
#pragma once

#include <queue>
#include <unordered_map>
#include <vector>

namespace Rain::Algorithm {
	// Computes the maximum flow on a graph using Edmonds-Karp in O(VE^2). The
	// provided graph must be simple (no self-loops, no multi-edges, connected).
	//
	// Returns the residual graph after the max flow is computed.
	std::
		pair<std::size_t, std::vector<std::unordered_map<std::size_t, std::size_t>>>
		maxFlowEdmondsKarp(
			std::vector<std::unordered_map<std::size_t, std::size_t>> const &edges,
			std::size_t const source,
			std::size_t const sink) {
		std::vector<std::unordered_map<std::size_t, std::size_t>> residual(edges);
		std::size_t flow{0};

		while (true) {
			// BFS for the shortest source-sink path.
			std::queue<std::size_t> bfsQueue;
			std::vector<std::size_t> parent(edges.size(), SIZE_MAX);
			bfsQueue.push(source);
			while (!bfsQueue.empty()) {
				std::size_t current{bfsQueue.front()};
				bfsQueue.pop();
				for (auto const &edge : residual[current]) {
					if (
						parent[edge.first] == SIZE_MAX &&
						residual[current][edge.first] > 0) {
						parent[edge.first] = current;
						bfsQueue.push(edge.first);
					}
				}
				if (parent[sink] != SIZE_MAX) {
					break;
				}
			}

			// Exit if no path found, otherwise update residuals.
			if (parent[sink] == SIZE_MAX) {
				break;
			}

			std::size_t pathFlow{SIZE_MAX};
			for (std::size_t current{sink}; current != source;
					 current = parent[current]) {
				pathFlow = std::min(pathFlow, residual[parent[current]][current]);
			}
			flow += pathFlow;
			for (std::size_t current{sink}; current != source;
					 current = parent[current]) {
				residual[parent[current]][current] -= pathFlow;
				residual[current][parent[current]] += pathFlow;
			}
		}

		// C++17: guaranteed either NRVO or move.
		return {flow, residual};
	}
}

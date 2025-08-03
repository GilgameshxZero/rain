#pragma once

#include <limits>
#include <unordered_map>
#include <vector>

namespace Rain::Algorithm {
	// Computes SSSP on a weighted, directed graph with SPFA in O(NM). The
	// provided graph must be simple and may contain negative weights.
	//
	// SPFA is a variation of Bellman-Ford which tracks vertices which need to be
	// considered for relaxation. Any vertices which are relaxed after the
	// `N-1`-th iteration must be reachable via a negative-weight cycle.
	//
	// Returns a list of minimal distances to each vertex, and the penultimate
	// vertex on the shortest path to each vertex. If a vertex is unreachable, the
	// distance is INF. If there exists any negative-weight cycle reachable from a
	// source, each negative-weight cycle will contain at least one vertex with
	// distance -INF.
	template <typename WeightType>
	inline std::pair<std::vector<WeightType>, std::vector<std::size_t>> ssspSpfa(
		std::vector<std::unordered_map<std::size_t, WeightType>> const &edges,
		std::vector<std::size_t> const &sources) {
		std::vector<WeightType> distances(
			edges.size(), std::numeric_limits<WeightType>::max());
		std::vector<std::size_t> predecessors(
			edges.size(), std::numeric_limits<std::size_t>::max());
		std::queue<std::pair<std::size_t, std::size_t>> queue;
		std::vector<std::size_t> enqueues(edges.size(), 0);
		for (auto const &i : sources) {
			distances[i] = 0;
			enqueues[i]++;
			queue.push({i, 0});
		}
		while (!queue.empty()) {
			auto [i, j]{queue.front()};
			queue.pop();
			enqueues[i]--;
			if (enqueues[i] > 0) {
				continue;
			}
			if (j == edges.size()) {
				distances[i] = std::numeric_limits<WeightType>::min();
				continue;
			}
			for (auto const &k : edges[i]) {
				if (distances[i] + k.second < distances[k.first]) {
					distances[k.first] = distances[i] + k.second;
					predecessors[k.first] = i;
					enqueues[k.first]++;
					queue.push({k.first, j + 1});
				}
			}
		}
		return {distances, predecessors};
	}
}
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
	// distance is INF. If a vertex is reachable via a negative-weight cycle, the
	// distance is -INF.
	template <typename IndexType, typename WeightType>
	std::pair<std::vector<WeightType>, std::vector<IndexType>> ssspSpfa(
		std::vector<std::unordered_map<IndexType, WeightType>> const &edges,
		IndexType const source) {
		std::vector<WeightType> distances(
			edges.size(), std::numeric_limits<WeightType>::max());
		distances[source] = 0;
		std::vector<IndexType> predecessors(
			edges.size(), std::numeric_limits<IndexType>::max());
		std::queue<std::pair<IndexType, IndexType>> queue;
		std::vector<bool> visited(edges.size(), true);
		queue.push({source, 0});
		visited[source] = false;
		while (!queue.empty()) {
			auto [i, j]{queue.front()};
			queue.pop();
			if (static_cast<std::size_t>(j) >= 2 * edges.size()) {
				break;
			}
			if (static_cast<std::size_t>(j) >= edges.size()) {
				distances[i] = std::numeric_limits<WeightType>::min();
			}
			if (visited[i]) {
				continue;
			}
			visited[i] = true;
			for (auto const &k : edges[i]) {
				if (
					distances[i] == std::numeric_limits<WeightType>::min() ||
					distances[i] + k.second < distances[k.first]) {
					if (distances[i] == std::numeric_limits<WeightType>::min()) {
						distances[k.first] = std::numeric_limits<WeightType>::min();
					} else {
						distances[k.first] = distances[i] + k.second;
					}
					predecessors[k.first] = i;
					visited[k.first] = false;
					queue.push({k.first, j + 1});
				}
			}
		}
		return {distances, predecessors};
	}
}
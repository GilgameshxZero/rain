#include <limits>
#include <unordered_map>
#include <vector>

namespace Rain::Algorithm {
	// Computes SSSP on directed weighted graph via Dijkstra's in $O(N + M\log
	// M)$, or until a target vertex is reached. The provided graph must be simple
	// and connected, and edge weights must be non-negative.
	//
	// Returns a list of distances to each node, and the penultimate node on the
	// shortest path to each node.
	template <typename IndexType, typename WeightType>
	std::pair<std::vector<WeightType>, std::vector<IndexType>> ssspDijkstra(
		std::vector<std::unordered_map<IndexType, WeightType>> const &edges,
		IndexType const source,
		IndexType const sink = std::numeric_limits<IndexType>::max()) {
		std::vector<WeightType> distances(
			edges.size(), std::numeric_limits<WeightType>::max());
		std::vector<IndexType> predecessors(
			edges.size(), std::numeric_limits<IndexType>::max());
		std::priority_queue<
			std::pair<WeightType, IndexType>,
			std::vector<std::pair<WeightType, IndexType>>,
			std::greater<std::pair<WeightType, IndexType>>>
			queue;
		std::vector<bool> visited(edges.size(), false);

		distances[source] = 0;
		queue.push({0, source});
		while (!queue.empty()) {
			auto i{queue.top().second};
			queue.pop();
			if (sink == i) {
				break;
			}
			if (visited[i]) {
				continue;
			}
			visited[i] = true;
			for (auto const &j : edges[i]) {
				if (visited[j.first]) {
					continue;
				}
				if (distances[i] + j.second < distances[j.first]) {
					distances[j.first] = distances[i] + j.second;
					predecessors[j.first] = i;
					queue.push({distances[j.first], j.first});
				}
			}
		}
		return {distances, predecessors};
	}
}
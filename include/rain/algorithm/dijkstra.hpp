#pragma once

#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace Rain::Algorithm {
	// Computes SSSP on directed weighted graph via Dijkstra's in $O(N + M\log
	// M)$, or until a target vertex is reached. The provided graph must be simple
	// and connected, and edge weights must be non-negative.
	//
	// Returns a list of distances to each node, and the penultimate node on the
	// shortest path to each node.
	template <typename WeightType>
	inline std::pair<std::vector<WeightType>, std::vector<std::size_t>>
	ssspDijkstra(
		std::vector<std::unordered_map<std::size_t, WeightType>> const &edges,
		std::size_t const source,
		std::size_t const sink = std::numeric_limits<std::size_t>::max()) {
		std::vector<WeightType> distances(
			edges.size(), std::numeric_limits<WeightType>::max());
		std::vector<std::size_t> predecessors(
			edges.size(), std::numeric_limits<std::size_t>::max());
		std::priority_queue<
			std::pair<WeightType, std::size_t>,
			std::vector<std::pair<WeightType, std::size_t>>,
			std::greater<std::pair<WeightType, std::size_t>>>
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

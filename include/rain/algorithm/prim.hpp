#pragma once

#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Rain::Algorithm {
	// Compute the minimum spanning tree (MST) of an undirected, weighted graph
	// with Prim's in O(N + M). The provided graph must be simple.
	//
	// Returns a subgraph without weights, along with the total cost of all the
	// weights, which is INF if a MST is not possible.
	template <typename WeightType>
	inline std::pair<std::vector<std::unordered_set<std::size_t>>, WeightType> mstPrim(
		std::vector<std::unordered_map<std::size_t, WeightType>> const &edges) {
		std::vector<std::unordered_set<std::size_t>> mst(edges.size());
		std::vector<bool> visited(edges.size(), false);
		std::priority_queue<
			std::pair<WeightType, std::pair<std::size_t, std::size_t>>,
			std::vector<std::pair<WeightType, std::pair<std::size_t, std::size_t>>>,
			std::greater<std::pair<WeightType, std::pair<std::size_t, std::size_t>>>>
			queue;
		WeightType cost{0};
		std::size_t cVisited{1};

		visited[0] = true;
		for (auto const &i : edges[0]) {
			queue.push({i.second, {0, i.first}});
		}
		while (!queue.empty()) {
			auto [w, i]{queue.top()};
			queue.pop();
			if (visited[i.second]) {
				continue;
			}
			visited[i.second] = true;
			cVisited++;
			mst[i.first].insert(i.second);
			cost += w;
			for (auto const &j : edges[i.second]) {
				if (visited[j.first]) {
					continue;
				}
				queue.push({j.second, {i.second, j.first}});
			}
		}
		return {
			mst,
			cVisited == edges.size() ? cost : std::numeric_limits<WeightType>::max()};
	}
}

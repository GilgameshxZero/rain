#include <deque>
#include <queue>
#include <unordered_set>
#include <vector>

namespace Rain::Algorithm {
	// Constructs an Eulerian cycle/circuit with Hierholzer's in O(E). The
	// provided graph must be unweighted and simple.
	//
	// Returns a list of points on the cycle. If the cycle is not constructable,
	// returns an empty list.
	inline std::vector<std::size_t> eulerCycleHeirholzer(
		std::vector<std::unordered_multiset<std::size_t>> const &edges,
		bool directed) {
		std::vector<std::size_t> inDegree(edges.size(), 0);
		std::size_t target{0};
		for (auto const &i : edges) {
			for (auto const &j : i) {
				inDegree[j]++;
			}
			target += i.size();
		}
		if (target == 0) {
			return {};
		}
		for (std::size_t i{0}; i < edges.size(); i++) {
			if (
				!directed && edges[i].size() % 2 != 0 ||
				directed && edges[i].size() != inDegree[i]) {
				return {};
			}
		}

		std::deque<std::size_t> cycle;
		std::size_t current;

		// Verify the connectivity of all edges via BFS.
		std::queue<std::size_t> queue;
		std::vector<bool> visited(edges.size(), false);
		std::size_t total{0};
		queue.push(0);
		while (edges[queue.front()].empty()) {
			queue.front()++;
		}
		cycle.push_back(queue.front());
		while (!queue.empty()) {
			current = queue.front();
			queue.pop();
			if (visited[current]) {
				continue;
			}
			visited[current] = true;
			total += edges[current].size();
			for (auto const &i : edges[current]) {
				queue.push(i);
			}
		}
		if (total != target) {
			return {};
		}
		if (!directed) {
			target /= 2;
		}

		auto residual{edges};
		while (cycle.size() < target) {
			while (residual[cycle.back()].empty()) {
				cycle.push_front(cycle.back());
				cycle.pop_back();
			}
			do {
				current = *residual[cycle.back()].begin();
				residual[cycle.back()].erase(residual[cycle.back()].begin());
				if (!directed) {
					residual[current].erase(residual[current].find(cycle.back()));
				}
				cycle.push_back(current);
			} while (!residual[cycle.back()].empty());
			if (cycle.front() == cycle.back()) {
				cycle.pop_back();
			}
		}

		return {cycle.begin(), cycle.end()};
	}
}
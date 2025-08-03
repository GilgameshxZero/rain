#include <deque>
#include <queue>
#include <unordered_set>
#include <vector>

namespace Rain::Algorithm {
	// Constructs an Eulerian cycle/circuit with Hierholzer's in O(E). The
	// provided graph must be unweighted, undirected, simple, and all edges must
	// be connected.
	//
	// Returns a list of points on the cycle. If the cycle is not constructable,
	// returns an empty list.
	inline std::vector<std::size_t> eulerCycleHeirholzer(
		std::vector<std::unordered_set<std::size_t>> const &edges) {
		std::size_t target{0};
		for (auto const &i : edges) {
			if (i.size() % 2 != 0) {
				return {};
			}
			target += i.size();
		}
		if (target == 0) {
			return {};
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
		target /= 2;

		auto residual{edges};
		while (cycle.size() < target) {
			while (residual[cycle.back()].empty()) {
				cycle.push_front(cycle.back());
				cycle.pop_back();
			}
			do {
				current = *residual[cycle.back()].begin();
				residual[cycle.back()].erase(current);
				residual[current].erase(cycle.back());
				cycle.push_back(current);
			} while (!residual[cycle.back()].empty());
			if (cycle.front() == cycle.back()) {
				cycle.pop_back();
			}
		}

		return {cycle.begin(), cycle.end()};
	}
}
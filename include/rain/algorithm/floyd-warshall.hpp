#include <limits>
#include <unordered_map>
#include <vector>

namespace Rain::Algorithm {
	// Computes APSP on directed weighted graph in $O(N^3)$. The provided graph
	// must be simple.
	//
	// Returns a matrix A, where A[i][j] is the shortest distance from node `i` to
	// node `j`.
	template <typename IndexType, typename WeightType>
	std::vector<std::vector<WeightType>> apspFloydWarshall(
		std::vector<std::unordered_map<IndexType, WeightType>> const &edges) {
		std::vector<std::vector<WeightType>> distances(
			edges.size(),
			std::vector<WeightType>(
				edges.size(), std::numeric_limits<WeightType>::max()));
		for (std::size_t i{0}; i < edges.size(); i++) {
			for (auto const &j : edges[i]) {
				distances[i][j.first] = j.second;
			}
			distances[i][i] = 0;
		}

		for (std::size_t k{0}; k < edges.size(); k++) {
			for (std::size_t i{0}; i < edges.size(); i++) {
				for (std::size_t j{0}; j < edges.size(); j++) {
					if (
						distances[i][k] == std::numeric_limits<WeightType>::max() ||
						distances[k][j] == std::numeric_limits<WeightType>::max()) {
						continue;
					}
					distances[i][j] =
						std::min(distances[i][j], distances[i][k] + distances[k][j]);
				}
			}
		}
		return distances;
	}
}
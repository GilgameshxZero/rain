#include <rain.hpp>

int main() {
	// Implements the example from
	// <https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm>.
	std::vector<std::unordered_set<std::size_t>> edges{
		{1}, {2}, {0}, {0, 1, 5}, {0, 6}, {3, 4}, {4}, {5, 6}};
	auto [cScc, scc]{Rain::Algorithm::sccTarjan(edges)};

	assert(cScc == 4);
	assert(scc[0] == scc[1]);
	assert(scc[0] == scc[2]);
	assert(scc[3] == scc[5]);
	assert(scc[4] == scc[6]);
	assert(scc[0] != scc[3]);
	assert(scc[0] != scc[4]);
	assert(scc[0] != scc[7]);
	assert(scc[3] != scc[4]);
	assert(scc[3] != scc[7]);
	assert(scc[4] != scc[7]);
	return 0;
}

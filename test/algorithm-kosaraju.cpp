#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	// Implements the example from
	// <https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm>.
	std::vector<std::unordered_set<std::size_t>> edges{
		{1}, {2}, {0}, {0, 1, 5}, {0, 6}, {3, 4}, {4}, {5, 6}};
	auto [cScc, scc]{Rain::Algorithm::sccKosaraju(edges)};

	releaseAssert(cScc == 4);
	releaseAssert(scc[0] == 3);
	releaseAssert(scc[1] == 3);
	releaseAssert(scc[2] == 3);
	releaseAssert(scc[3] == scc[5]);
	releaseAssert(scc[4] == scc[6]);
	releaseAssert(scc[7] == 0);
	releaseAssert(scc[0] != scc[3]);
	releaseAssert(scc[0] != scc[4]);
	releaseAssert(scc[0] != scc[7]);
	releaseAssert(scc[3] != scc[4]);
	releaseAssert(scc[3] != scc[7]);
	releaseAssert(scc[4] != scc[7]);
	return 0;
}

#include <rain.hpp>

int main() {
	// Implements the example in the Wikipedia article: <https://en.wikipedia.org/wiki/Edmonds%E2%80%93Karp_algorithm>.
	std::vector<std::unordered_map<std::size_t, std::size_t>> G(7);
	G[0][1] = 3;
	G[0][3] = 3;
	G[1][2] = 4;
	G[2][0] = 3;
	G[2][3] = 1;
	G[2][4] = 2;
	G[3][4] = 2;
	G[3][5] = 6;
	G[4][1] = 1;
	G[4][6] = 1;
	G[5][6] = 9;
	auto[flow, R]{Rain::Algorithm::maxFlowEdmondsKarp(G, 0, 6)};
	assert(flow == 5);
	assert(R[0][3] == 0);
	assert(R[0][1] == 1);
	assert(R[1][2] == 2);
	assert(R[2][3] == 0);
	assert(R[2][4] == 1);
	assert(R[3][5] == 2);
	assert(R[4][6] == 0);
	assert(R[5][6] == 5);
	return 0;
}

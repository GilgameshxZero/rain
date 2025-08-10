#include <rain/functional.hpp>

#include <array>
#include <cassert>

using namespace Rain;
using namespace Functional;
using namespace std;

int main() {
	// Must include <array>.
	assert((Rain::Functional::isStdHashable<long long>()).value);
	assert(!(Rain::Functional::isStdHashable<std::array<int, 8>>()).value);
	assert(!(Rain::Functional::isBaseOfTemplate<std::pair, std::array<int, 8>>())
						.value);
	assert((Rain::Functional::isConstIterable<std::array<int, 8>>()).value);

	// Cannot use std::is_base_of with template base types.
	assert((isBaseOfTemplate<pair, pair<int, char>>()).value);
	assert(!(isBaseOfTemplate<vector, pair<int, char>>()).value);
	assert((isBaseOfTemplate<vector, vector<int>>()).value);
	return 0;
}

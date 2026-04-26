#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	// Must include <array>.
	releaseAssert(
		(Rain::Functional::isStdHashable<long long>()).value);
	releaseAssert(
		!(Rain::Functional::isStdHashable<std::array<int, 8>>())
			.value);
	releaseAssert(
		!(Rain::Functional::
				isBaseOfTemplate<std::pair, std::array<int, 8>>())
			.value);
	releaseAssert((
		Rain::Functional::isConstIterable<std::array<int, 8>>())
			.value);

	// Cannot use std::is_base_of with template base types.
	releaseAssert(
		(Rain::Functional::
				isBaseOfTemplate<std::pair, std::pair<int, char>>())
			.value);
	releaseAssert(!(
		Rain::Functional::
			isBaseOfTemplate<std::vector, std::pair<int, char>>())
			.value);
	releaseAssert(
		(Rain::Functional::
				isBaseOfTemplate<std::vector, std::vector<int>>())
			.value);
	return 0;
}

// Tests for Rain::Filesystem.
#include <rain/filesystem.hpp>

#include <cassert>
#include <iostream>
#include <unordered_map>

int main() {
	std::filesystem::path workingDir{std::filesystem::current_path()};
	std::cout << "Working directory: " << workingDir << std::endl;

	std::filesystem::path ancestorDir{workingDir / "../../../"};
	std::cout << "Ancestor directory: " << ancestorDir << std::endl;
	assert(Rain::Filesystem::isSubpath(workingDir, ancestorDir));

	// Ensure hash operator works.
	std::unordered_map<std::filesystem::path, int> um;
	um[workingDir] = 3;

	return 0;
}

#pragma once

#include <filesystem>

namespace Rain::Filesystem {
	// Returns true if needle is under the haystack directory path.
	// Safest with absolute paths.
	inline bool subpath(std::filesystem::path const &haystack,
		std::filesystem::path const &needle) {
		return std::mismatch(
						 needle.begin(), needle.end(), haystack.begin(), haystack.end())
						 .second == haystack.end();
	}
}

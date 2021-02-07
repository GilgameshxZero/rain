#pragma once

#include <filesystem>

namespace Rain::Filesystem {
	// Returns true if needle is under the haystack directory path.
	// Safest with absolute paths.
	inline bool subpath(const std::filesystem::path &haystack,
		const std::filesystem::path &needle) {
		return std::mismatch(
						 needle.begin(), needle.end(), haystack.begin(), haystack.end())
						 .second == haystack.end();
	}
}

// Utilities for std::filesystem.
#pragma once

#include <filesystem>

namespace Rain::Filesystem {
	// Returns true if descendent path is under the directory subtree of the
	// ancestor path.
	//
	// Does not require canonical paths.
	inline bool isSubpath(
		std::filesystem::path const &descendant,
		std::filesystem::path const &ancestor) {
		std::filesystem::path canonDescendant(
			std::filesystem::canonical(descendant)),
			canonAncestor(std::filesystem::canonical(ancestor));
		return std::mismatch(
						 canonDescendant.begin(),
						 canonDescendant.end(),
						 canonAncestor.begin(),
						 canonAncestor.end())
						 .second == canonAncestor.end();
	}
}

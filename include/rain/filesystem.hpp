// Utilities for std::filesystem.
#pragma once

#include <filesystem>
#include <fstream>

namespace Rain::Filesystem {
	// Returns true if descendent path is under the directory subtree of the
	// ancestor path. This does not traverse symbolic links (though, it will
	// traverse initial symbolic links given to `descendant` and `ancestor` via
	// `canonical`).
	//
	// Internally converts to canonical or weakly canonical paths. Note that
	// absolute paths do not remove `..` elements in paths.
	inline bool isSubpath(
		std::filesystem::path const &descendant,
		std::filesystem::path const &ancestor) {
		std::filesystem::path trueDescendant(
			std::filesystem::canonical(descendant)),
			trueAncestor(std::filesystem::canonical(ancestor));
		return std::mismatch(
						 trueDescendant.begin(),
						 trueDescendant.end(),
						 trueAncestor.begin(),
						 trueAncestor.end())
						 .second == trueAncestor.end();
	}

	inline bool compareFiles(
		std::filesystem::path const &firstPath,
		std::filesystem::path const &secondPath) {
		std::ifstream first(firstPath, std::ifstream::binary | std::ifstream::ate);
		std::ifstream second(
			secondPath, std::ifstream::binary | std::ifstream::ate);
		if (first.fail() || second.fail()) {
			return false;
		}
		if (first.tellg() != second.tellg()) {
			return false;
		}

		first.seekg(0, std::ifstream::beg);
		second.seekg(0, std::ifstream::beg);
		return std::equal(
			std::istreambuf_iterator<char>(first.rdbuf()),
			std::istreambuf_iterator<char>(),
			std::istreambuf_iterator<char>(second.rdbuf()));
	}
}

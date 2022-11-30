// Utilities for std::filesystem.
#pragma once

#include <filesystem>
#include <fstream>

namespace Rain::Filesystem {
	// Hash std::filesystem::path.
	struct HashPath {
		std::size_t operator()(std::filesystem::path const &path) const {
			return std::hash<std::string>{}(path.string());
		}
	};

	// Returns true if descendent path is under the directory subtree of the
	// ancestor path.
	//
	// Internally converts to canonical or absolute paths.
	inline bool isSubpath(
		std::filesystem::path const &descendant,
		std::filesystem::path const &ancestor, bool useCanonical = true) {
		std::filesystem::path trueDescendant(useCanonical ? 
			std::filesystem::canonical(descendant) : std::filesystem::absolute(descendant)),
			trueAncestor(useCanonical ? std::filesystem::canonical(ancestor) : std::filesystem::absolute(ancestor));
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

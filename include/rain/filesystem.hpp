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
	// Internally converts to canonical paths.
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

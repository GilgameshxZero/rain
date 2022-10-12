// Tests the Knuth-Morris-Pratt string matching algorithm from
// `rain/algorithm/kmp.hpp`.
#include <rain/algorithm/kmp.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main() {
	using namespace Rain::Literal;

	// Partial table computation.
	std::string s{"PARTICIPATE IN PARACHUTE"};
	std::cout << "String: " << s << std::endl;

	std::vector<std::size_t> partialMatch =
		Rain::Algorithm::computeKmpPartialMatch(s);
	std::cout << "Partial match table: ";
	for (size_t i{0}; i < partialMatch.size(); i++) {
		std::cout << partialMatch[i] << " ";
	}
	assert(
		partialMatch ==
		std::vector<std::size_t>({SIZE_MAX, 0, 0, 0, 0, 0, 0,				 SIZE_MAX, 0,
															2,				0, 0, 0, 0, 0, SIZE_MAX, 0,				 0,
															3,				0, 0, 0, 0, 0, 0}));

	// Partial table computation.
	s = "\r\n";
	partialMatch = Rain::Algorithm::computeKmpPartialMatch(s.c_str(), s.length());
	std::cout << std::endl << "Partial match table for \\r\\n: ";
	for (size_t i{0}; i < partialMatch.size(); i++) {
		std::cout << partialMatch[i] << " ";
	}
	assert(partialMatch == std::vector<std::size_t>({SIZE_MAX, 0, 0}));

	// Partial table computation.
	s = "\r\n\r\n";
	partialMatch = Rain::Algorithm::computeKmpPartialMatch(s.c_str(), s.length());
	std::cout << std::endl << "Partial match table for \\r\\n\\r\\n: ";
	for (size_t i{0}; i < partialMatch.size(); i++) {
		std::cout << partialMatch[i] << " ";
	}
	assert(
		partialMatch == std::vector<std::size_t>({SIZE_MAX, 0, SIZE_MAX, 0, 2}));

	// String search.
	s = "ABC ABCDAB ABCDABCDABDE";
	std::string w{"ABCDABD"};
	std::string match =
		s.substr(Rain::Algorithm::kmpSearch(s, w).first, w.length());
	std::string matchStr(match, w.length());
	std::cout << std::endl
						<< "String: " << s << std::endl
						<< "Search result: " << match << std::endl;
	assert(w == match);

	// String search with multiple matches.
	auto m = Rain::Algorithm::kmpSearch(s, "BC");
	assert(m.first == 1_zu);
	assert(m.second == 2_zu);

	// String search with no match.
	m = Rain::Algorithm::kmpSearch(s, "CBAD");
	assert(m.first >= s.length());
	assert(m.second == 0_zu);
	return 0;
}

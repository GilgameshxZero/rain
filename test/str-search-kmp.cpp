/*
Tests the Knuth-Morris-Pratt string matching algorithm from
`rain/algorithm/kmp.hpp`.
*/

#include <rain/algorithm/kmp.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <iostream>
#include <vector>

int main() {
	using namespace Rain::Literal;

	std::string s = "PARTICIPATE IN PARACHUTE";
	std::vector<std::size_t> partialMatch(s.length() + 1);

	std::cout << "String: " << s << std::endl;
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << "Partial match table: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}
	assert(partialMatch ==
		std::vector<std::size_t>({std::numeric_limits<std::size_t>::max(),
			0,
			0,
			0,
			0,
			0,
			0,
			std::numeric_limits<std::size_t>::max(),
			0,
			2,
			0,
			0,
			0,
			0,
			0,
			std::numeric_limits<std::size_t>::max(),
			0,
			0,
			3,
			0,
			0,
			0,
			0,
			0,
			0}));

	s = "\r\n";
	partialMatch.resize(s.length() + 1);
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << std::endl << "Partial match table for \\r\\n: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}
	assert(partialMatch ==
		std::vector<std::size_t>({std::numeric_limits<std::size_t>::max(), 0, 0}));

	s = "\r\n\r\n";
	partialMatch.resize(s.length() + 1);
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << std::endl << "Partial match table for \\r\\n\\r\\n: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}
	assert(partialMatch ==
		std::vector<std::size_t>({std::numeric_limits<std::size_t>::max(),
			0,
			std::numeric_limits<std::size_t>::max(),
			0,
			2}));

	s = "ABC ABCDAB ABCDABCDABDE";
	std::string w = "ABCDABD";
	char *match = Rain::Algorithm::cStrSearchKmp(
		s.c_str(), s.length(), w.c_str(), w.length());
	std::string matchStr(match, w.length());
	std::cout << std::endl
						<< "String: " << s << std::endl
						<< "Search result: " << std::string(match - s.c_str(), ' ') << matchStr
						<< std::endl;
	assert(w == matchStr);
	return 0;
}

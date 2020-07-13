#include "rain.hpp"

int main() {
	std::string s = "PARTICIPATE IN PARACHUTE ";
	std::vector<size_t> partialMatch(s.length() + 1);

	std::cout << "String: " << s << std::endl;
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << "Partial match table: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}

	s = "\r\n";
	partialMatch.resize(s.length() + 1);
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << std::endl << "Partial match table for \\r\\n: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}

	s = "\r\n\r\n";
	partialMatch.resize(s.length() + 1);
	Rain::Algorithm::computeKmpPartialMatch(
		s.c_str(), s.length(), partialMatch.data());
	std::cout << std::endl << "Partial match table for \\r\\n\\r\\n: ";
	for (size_t a = 0; a < partialMatch.size(); a++) {
		std::cout << partialMatch[a] << " ";
	}

	s = "ABC ABCDAB ABCDABCDABDE";
	std::string w = "ABCDABD";
	char *match = Rain::Algorithm::cStrSearchKMP(
		s.c_str(), s.length(), w.c_str(), w.length());
	std::cout << std::endl
						<< "String: " << s << std::endl
						<< "Result: " << std::string(match - s.c_str(), ' ') << w
						<< std::endl;
	return 0;
}

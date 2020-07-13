#include <rain.hpp>

int main() {
	std::string str = "WhAT? iS? thIs?";
	std::cout << "Original string: " << str << "\n"
						<< "toLowerStr:      " << *Rain::String::toLowerStr(&str) << "\n";

	char cStr[] = "Hello world!! This is Yang :)";
	std::cout << "Original string: " << cStr << "\n"
						<< "toLowerCStr:     " << Rain::String::toLowerCStr(cStr) << "\n";

	std::string whiteStr = "   	\t	\nSome string with whitespace\t\t\n   	";
	std::cout << "Original string:   " << whiteStr << "\n"
						<< "trimWhitespaceStr: "
						<< *Rain::String::trimWhitespaceStr(&whiteStr) << "\n";

	char whiteCStr[] = "   	\t	\nSome string with whitespace\t\t\n   	";
	std::cout << "Original string:             " << whiteCStr << "\n";
	// Terminate the c-string to cut off the last whitespace.
	*const_cast<char *>(
		Rain::String::findFirstNonWhitespaceCStrN(
			whiteCStr + sizeof(whiteCStr) - 2, sizeof(whiteCStr), -1) +
		1) = '\0';
	std::cout << "findFirstNonWhitespaceCStrN: "
						<< Rain::String::findFirstNonWhitespaceCStrN(whiteCStr) << "\n";

	const char *numCStr = "-455.30";
	std::cout << "String:          " << numCStr << "\n"
						<< "anyToAny int:    " << Rain::String::anyToAny<int>(numCStr)
						<< "\n"
						<< "anyToAny double: " << Rain::String::anyToAny<double>(numCStr)
						<< "\n";
	double dblNum = -38.1415;
	std::cout << "Double:          " << dblNum << "\n"
						<< "anyToAny string: "
						<< Rain::String::anyToAny<std::string>(dblNum) << "\n";

	return 0;
}

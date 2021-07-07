/*
Tests select string parsing utilities from `rain/string.hpp`.
*/

#include <rain/string.hpp>

#include <cassert>
#include <iostream>

int main() {
	std::string str = "WhAT? iS? thIs?";
	std::cout << "Original string: " << str << "\n"
						<< "toLowerStr:      " << *Rain::String::toLowerStr(&str) << "\n";
	assert(str == "what? is? this?");

	char cStr[] = "Hello world!! This is Yang :)";
	std::cout << "Original string: " << cStr << "\n"
						<< "toLowerCStr:     " << Rain::String::toLowerCStr(cStr) << "\n";
	assert(strcmp(cStr, "hello world!! this is yang :)") == 0);

	std::string whiteStr = "   	\t	\nSome string with whitespace\t\t\n   	";
	std::cout << "Original string:   " << whiteStr << "\n"
						<< "trimWhitespaceStr: "
						<< *Rain::String::trimWhitespaceStr(&whiteStr) << "\n";
	assert(whiteStr == "Some string with whitespace");

	char whiteCStr[] = "   	\t	\nSome string with whitespace\t\t\n   	";
	char *firstNonWhitespace =
		Rain::String::findFirstNonWhitespaceCStrN(whiteCStr);
	std::cout << "Original string:             " << whiteCStr << "\n";
	std::cout << "findFirstNonWhitespaceCStrN: " << firstNonWhitespace << "\n";
	assert(
		strcmp(firstNonWhitespace, "Some string with whitespace\t\t\n   	") == 0);

	const char *numCStr = "-455.30";
	int i = Rain::String::anyToAny<int>(numCStr);
	double d = Rain::String::anyToAny<double>(numCStr);
	std::cout << "String:          " << numCStr << "\n"
						<< "anyToAny int:    " << i << "\n"
						<< "anyToAny double: " << d << "\n";
	assert(i == -455);
	assert(d == -455.3);

	double dblNum = -38.1415;
	std::string s = Rain::String::anyToAny<std::string>(dblNum);
	std::cout << "Double:          " << dblNum << "\n"
						<< "anyToAny string: " << s << "\n";
	assert(s == "-38.1415");

	return 0;
}

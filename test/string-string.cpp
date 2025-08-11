// Tests string utilities from String.
#include <rain.hpp>

int main() {
	std::string str{"WhAT? iS? thIs?"};
	std::cout << "Original string: " << str << "\n"
						<< "toLowerStr:      " << Rain::String::toLower(str) << "\n";
	assert(str == "what? is? this?");

	char cStr[] = "Hello world!! This is Yang :)";
	std::cout << "Original string: " << cStr << "\n"
						<< "toLowerCStr:     " << Rain::String::toLower(cStr, 29) << "\n";
	assert(strcmp(cStr, "hello world!! this is yang :)") == 0);

	std::string whiteStr{"   	\t	\nSome string with whitespace\t\t\n   	"};
	std::cout << "Original string:   " << whiteStr << "\n"
						<< "trimWhitespaceStr: " << Rain::String::trimWhitespace(whiteStr)
						<< "\n";
	assert(whiteStr == "Some string with whitespace");

	char whiteCStr[]{"   	\t	\nSome string with whitespace\t\t\n   	"};
	char *firstNonWhitespace =
		Rain::String::scanUntilNonWhitespace(whiteCStr, 41);
	std::cout << "Original string:             " << whiteCStr << "\n";
	std::cout << "findFirstNonWhitespaceCStrN: " << firstNonWhitespace << "\n";
	assert(
		strcmp(firstNonWhitespace, "Some string with whitespace\t\t\n   	") == 0);

	const char *numCStr = "-455.30";
	int i{Rain::String::anyToAny<int>(numCStr)};
	long double d{Rain::String::anyToAny<long double>(numCStr)};
	std::cout << "String:          " << numCStr << "\n"
						<< "anyToAny int:    " << i << "\n"
						<< "anyToAny long double: " << d << "\n";
	assert(i == -455);
	assert(d == -455.3l);

	long double dblNum{-38.1415};
	std::string s{Rain::String::anyToAny<std::string>(dblNum)};
	std::cout << "long double:          " << dblNum << "\n"
						<< "anyToAny string: " << s << "\n";
	assert(s == "-38.1415");

	{
		std::unordered_set<
			std::string,
			Rain::String::CaseAgnosticHash,
			Rain::String::CaseAgnosticEqual>
			S;
		S.insert("hello world");
		S.insert("Hello World");
		assert(S.size() == 1);
	}

	return 0;
}

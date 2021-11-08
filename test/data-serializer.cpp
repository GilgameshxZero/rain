// Tests for pickle serialization/deserialization.
#include <rain/data/serializer.hpp>
#include <rain/literal.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
	using namespace Rain::Literal;

	int sData[]{1, 4, 5, 10, -5, -49, 3049};
	std::string sStr{"yahallo!"};
	std::vector<long long> sVrLL{439548549457045LL, 348LL, 3934957LL, 39LL, -4LL};
	std::vector<std::string> sVrStr{"hello", "world", "! :D"};
	{
		Rain::Data::Serializer serializer("data-serializer.txt");
		serializer << sData << sStr << sVrLL << sVrStr;
	}

	decltype(sData) dData;
	decltype(sStr) dStr;
	decltype(sVrLL) dVrLL;
	decltype(sVrStr) dVrStr;
	Rain::Data::Deserializer deserializer("data-serializer.txt");
	deserializer >> dData >> dStr >> dVrLL >> dVrStr;

	assert(memcmp(sData, dData, sizeof(sData)) == 0);
	assert(sStr == dStr);
	assert(sVrLL == dVrLL);
	assert(sVrStr == dVrStr);

	return 0;
}

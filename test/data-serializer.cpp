// Tests for pickle serialization/deserialization.
#include <rain.hpp>

using namespace Rain;
using namespace Data;
using namespace Error;

using namespace std;

int main() {
	{
		static_assert(Serializer::HasSpec<std::string>::value);
		static_assert(!Serializer::HasSpec<int>::value);
		static_assert(
			Deserializer::HasSpec<std::string>::value);
		static_assert(!Deserializer::HasSpec<int>::value);
	}

	int sData[]{1, 4, 5, 10, -5, -49, 3049};
	std::string sStr{"yahallo!"};
	std::vector<long long> sVrLL{
		439548549457045LL, 348LL, 3934957LL, 39LL, -4LL};
	std::vector<std::string> sVrStr{"hello", "world", "! :D"};
	{
		ofstream file("data-serializer.txt");
		Serializer serializer(file);
		serializer << sData << sStr << sVrLL << sVrStr;
	}

	decltype(sData) dData;
	decltype(sStr) dStr;
	decltype(sVrLL) dVrLL;
	decltype(sVrStr) dVrStr;
	ifstream file("data-serializer.txt");
	Deserializer deserializer(file);
	deserializer >> dData >> dStr >> dVrLL >> dVrStr;

	releaseAssert(memcmp(sData, dData, sizeof(sData)) == 0);
	releaseAssert(sStr == dStr);
	releaseAssert(sVrLL == dVrLL);
	releaseAssert(sVrStr == dVrStr);

	return 0;
}

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
	int *sPData = &sData[3];
	int **sPPData = &sPData;

	{
		Rain::Data::Serializer serializer("data-serializer.txt");
		serializer << sData << sPData << sPPData;
	}

	decltype(sData) dData;
	decltype(sPData) dPData;
	decltype(sPPData) dPPData;

	Rain::Data::Serializer serializer("data-serializer.txt");
	serializer >> dData >> dPData >> dPPData;
	assert(memcmp(sData, dData, sizeof(sData)) == 0);
	assert(dPData == dData + 3);
	assert(*dPPData == dPData);
	dData[3] += 93;
	assert(*dPData == 103);

	return 0;
}

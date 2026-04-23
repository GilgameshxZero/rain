#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	{
		using namespace Rain::Math;
		releaseAssert((IsPrime<5>()).value);
		releaseAssert((IsPrime<1009>()).value);
		releaseAssert(!(IsPrime<1010>()).value);
	}
	return 0;
}

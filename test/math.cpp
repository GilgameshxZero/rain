#include <rain/math.hpp>

#include <cassert>

int main() {
	{
		using namespace Rain::Math;
		assert((IsPrime<5>()).value);
		assert((IsPrime<1009>()).value);
		assert(!(IsPrime<1010>()).value);
	}
	return 0;
}

/*
User-defined literals. Literals are meant to be resolved through the namespace
scope: `using namespace Rain::Literal;`.
*/

#pragma once

namespace Rain::Literal {
	/*
	User-defined literals for std::size_t.
	*/
	std::size_t operator"" _zu(unsigned long long);
}

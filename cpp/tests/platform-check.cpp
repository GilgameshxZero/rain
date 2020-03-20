#include "../../utility-platform.hpp"

// Conditionally include a windows header.
#include "../../windows-lam-include.hpp"

#include <iostream>

int main() {
	std::cout << "Your platform is " << Rain::getPlatformString() << ".\n";
	return 0;
}

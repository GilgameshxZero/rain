#include "time.hpp"

#include <iostream>

int main() {
	std::cout << "The current time is " << Rain::getTime() << ".\n"
						<< "Sleeping for 2 seconds...\n";
	Rain::sleep(2000);
	std::cout << "The current time is " << Rain::getTime() << ".\n";
	return 0;
}

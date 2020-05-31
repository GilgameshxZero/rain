#include "rain.hpp"

#include <iostream>

int main() {
	std::cout << "Using rain v"
						<< RAIN_VERSION_MAJOR << "."
						<< RAIN_VERSION_MINOR << "."
						<< RAIN_VERSION_REVISION << "."
						<< RAIN_VERSION_BUILD << "." << Rain::CRLF;
	std::cout << "This binary was built on " << Rain::getPlatformString() << "." << Rain::CRLF;
	return 0;
}

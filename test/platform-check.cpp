/*
Prints the `rain` version, and information about the build and platform.

Tests including all the `rain` headers.
*/

#include <rain.hpp>

int main() {
	std::cout << "rain v" << RAIN_VERSION_MAJOR << "." << RAIN_VERSION_MINOR
						<< "." << RAIN_VERSION_REVISION << "." << RAIN_VERSION_BUILD << "."
						<< std::endl;
	std::cout << "Debug: " << (Rain::Platform::isDebug() ? "Yes." : "No.")
						<< std::endl;
	std::cout << "Platform: " << Rain::Platform::getPlatformCStr() << "."
						<< std::endl;
	return 0;
}

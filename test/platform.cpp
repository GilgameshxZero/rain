// Tests for Rain::Platform.
#include <rain/platform.hpp>

#include <iostream>

int main() {
	std::cout << "Debug: " << (Rain::Platform::isDebug() ? "Yes." : "No.")
						<< std::endl;
	std::cout << "Platform: " << Rain::Platform::getPlatform() << "."
						<< std::endl;
	return 0;
}

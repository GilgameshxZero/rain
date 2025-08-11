// Tests for Rain::Platform.
#include <rain.hpp>

int main() {
	std::cout << "Debug: " << (Rain::Platform::isDebug() ? "Yes." : "No.")
						<< std::endl;
	std::cout << "Platform: " << Rain::Platform::getPlatform() << "."
						<< std::endl;
	std::cout << "RAIN_VERSION_BUILD: " << RAIN_VERSION_BUILD << std::endl;
	return 0;
}

#include "rain.hpp"

int main() {
	Rain::Networking::Server server;
	server.acceptTimeoutMs = 10000;
	server.onBeginSlaveTask = [](Rain::Networking::Server::Slave *slave) {
		std::cout << "onBeginSlaveTask\n";
	};
	server.onCloseSlave = [](Rain::Networking::Server::Slave *slave) {
		std::cout << "onCloseSlave\n";
	};
	std::cout << "Starting server...\n";
	server.serve(Rain::Networking::Host(NULL, 80), false);
	Rain::Time::sleepMs(10000);
	std::cout << "Stopping server...\n";
	server.close();

	Rain::Networking::Socket::cleanup();
	return 0;
}

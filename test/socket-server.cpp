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
	server.serve(Rain::Networking::Host(NULL, 0), false);
	std::cout << "Serving on port " << server.getService().getCStr() << "\n";
	Rain::Time::sleepMs(10000);
	std::cout << "Stopping server...\n";
	return 0;
}

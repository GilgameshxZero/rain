#include "rain.hpp"

int main() {
	Rain::Networking::Http::Server server;
	server.onBeginSlaveTask = [](Rain::Networking::Http::Server::Slave *slave) {
		std::cout << "onBeginSlaveTask\n";
	};
	server.onCloseSlave = [](Rain::Networking::Http::Server::Slave *slave) {
		std::cout << "onCloseSlave\n";
	};
	server.onRequest = [](Rain::Networking::Http::Server::Request *req) {
		std::cout << req->method << " " << req->uri << "\n";
		Rain::Networking::Http::Server::Response res(req->slave);
		res.headers["content-type"] = "text/html";
		res.headers["server"] = "emilia/rain";
		res.setBody("hi");
		res.send();
	};
	std::cout << "Starting server...\n";
	server.serve(Rain::Networking::Host(NULL, 80), false);
	Rain::Time::sleep(30000);
	std::cout << "Stopping server...\n";
	server.close();

	Rain::Networking::Socket::cleanup();
	return 0;
}

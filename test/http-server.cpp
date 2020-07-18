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
		res.header["Content-Type"] = "text/html";
		res.header["Server"] = "emilia/rain";
		res.body.appendBytes("hihi!<br>");
		res.body.appendBytes("a second set of bytes");

		try {
			res.send();
		} catch (...) {
			std::cout << "Failed to send response.\n";
			throw;
		}
	};
	std::cout << "Starting server...\n";
	server.serve(Rain::Networking::Host(NULL, 80), false);
	Rain::Time::sleep(10000);
	std::cout << "Stopping server...\n";
	server.close();

	Rain::Networking::Socket::cleanup();
	return 0;
}

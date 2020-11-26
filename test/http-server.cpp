#include "rain.hpp"

int main() {
	typedef Rain::Networking::Http::Server<void *> Server;
	Server server;
	server.acceptTimeoutMs = 10000;
	server.onBeginSlaveTask = [](Server::Slave *slave) {
		std::cout << "onBeginSlaveTask\n";
	};
	server.onCloseSlave = [](Server::Slave *slave) {
		std::cout << "onCloseSlave\n";
	};
	server.onRequest = [](Server::Request *req) {
		std::cout << req->method << " " << req->path << "\n";
		Server::Response res(req->slave);
		res.body.appendBytes("hihi!<br>");
		res.body.appendBytes("a second set of bytes");
		res.header["Content-Type"] = "text/html";
		res.header["Server"] = "emilia/rain";
		res.header["Content-Length"] = std::to_string(res.body.getBytesLength());

		try {
			res.send();
		} catch (...) {
			std::cout << "Failed to send response.\n";
			throw;
		}
	};
	std::cout << "Starting server...\n";
	server.serve(Rain::Networking::Host(NULL, 0), false);
	Rain::Time::sleepMs(10000);
	std::cout << "Stopping server...\n";
	server.close();

	Rain::Networking::Socket::cleanup();
	return 0;
}

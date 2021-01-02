#include <rain/networking/http/server.hpp>
#include <rain/networking/http/slave.hpp>

#include <iostream>

class Server
		: public Rain::Networking::Http::Server<Rain::Networking::Http::Slave> {
	protected:
	bool onRequest(Slave &slave, Request &req) noexcept override {
		std::cout << req.method << " " << req.path << "\n";
		Server::Response res;
		res.body.appendBytes("hihi!<br>");
		res.body.appendBytes("a second set of bytes");
		res.header["Content-Type"] = "text/html";
		res.header["Server"] = "emilia/rain";
		res.header["Content-Length"] = std::to_string(res.body.getBytesLength());
		return slave.send(res);
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host("*", 0), false);
	std::cout << "Started server on port " << server.getService().getCStr()
						<< ".\n";
	Rain::Time::sleepMs(15000);
	std::cout << "Stopping server...\n";
	return 0;
}

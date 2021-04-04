#include <rain/networking/http/server.hpp>
#include <rain/networking/http/slave.hpp>
#include <rain/networking/socket.hpp>

#include <iostream>

class Server
		: public Rain::Networking::Http::Server<Rain::Networking::Http::Slave> {
	protected:
	bool onRequest(Slave &slave, Request &req) noexcept override {
		std::cout << req.method << " " << req.path << " HTTP/" << req.version << std::endl;

		Server::Response res;
		res.body.appendBytes("hihi!<br>");
		res.body.appendBytes("a second set of bytes");
		res.header["Content-Type"] = "text/html";
		res.header["Server"] = "emilia/rain";
		return slave.send(res);
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host("localhost", 0), false);
	std::cout << "Started server on port " << server.getService().getCStr()
						<< std::endl;

	Rain::Networking::Socket socket;
	socket.connect(Rain::Networking::Host("localhost", server.getService()));
	socket.send("GET / HTTP/1.1\r\n\r\n");
	Rain::Time::sleepMs(1000);
	char buf[1024];
	std::size_t len = socket.recv(buf, 1024);
	buf[len] = '\0';
	std::cout << buf << std::endl;

	std::cout << "Stopping server..." << std::endl;

	return 0;
}

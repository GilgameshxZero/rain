#include <rain/networking/http/server.hpp>

#include <iostream>

class Server;

class ServerSlave
		: public Rain::Networking::Http::ServerSlave<Server, ServerSlave, void *> {
	public:
	typedef Rain::Networking::Http::ServerSlave<Server, ServerSlave, void *>
		ServerSlaveBase;
	ServerSlave(Rain::Networking::Socket &socket, Server *server)
			: Rain::Networking::Socket(std::move(socket)),
				ServerSlaveBase(socket, server) {}
};

class Server : public Rain::Networking::Http::Server<ServerSlave> {
	public:
	typedef Rain::Networking::Http::Server<ServerSlave> ServerBase;
	Server(std::size_t maxThreads = 0, std::size_t slaveBufSz = 16384)
			: ServerBase(maxThreads, slaveBufSz) {}

	protected:
	void *getSubclassPtr() { return reinterpret_cast<void *>(this); }
	void onRequest(Request *req) {
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
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host(NULL, 0), false);
	std::cout << "Started server on port " << server.getService().getCStr()
						<< ".\n";
	Rain::Time::sleepMs(10000);
	std::cout << "Stopping server...\n";
	return 0;
}

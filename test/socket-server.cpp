#include <rain/networking/client.hpp>
#include <rain/networking/server.hpp>

#include <iostream>

class Server;

class ServerSlave
		: public Rain::Networking::ServerSlave<Server, ServerSlave, void *> {
	public:
	typedef Rain::Networking::ServerSlave<Server, ServerSlave, void *>
		ServerSlaveBase;
	ServerSlave(Rain::Networking::Socket &socket, Server *server)
			: Rain::Networking::Socket(std::move(socket)),
				ServerSlaveBase(socket, server) {}
};

class Server : public Rain::Networking::Server<ServerSlave> {
	public:
	typedef Rain::Networking::Server<ServerSlave> ServerBase;
	Server(std::size_t maxThreads = 0) : ServerBase(maxThreads) {}

	protected:
	void *getSubclassPtr() { return reinterpret_cast<void *>(this); }
	void onSlaveTask(ServerSlave *slave) {
		std::cout << "Spawned slave " << slave->getNativeSocket()
							<< ". Receiving...\n";
		try {
			char buffer[1024];
			std::size_t recvLen;
			while (true) {
				recvLen = slave->recv(buffer,
					sizeof(buffer),
					Rain::Networking::Socket::RecvFlag::NONE,
					std::chrono::milliseconds(500));
				std::cout << "Received " << recvLen
									<< " bytes: " << std::string(buffer, recvLen) << "\n";
			}
		} catch (const std::exception &e) {
			std::cout << "Receiving terminated by exception.\n" << e.what() << "\n";
		}
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host(NULL, 0), false);
	Rain::Networking::Host host("127.0.0.1", server.getService());
	std::cout << "Started server on port " << host.service.getCStr() << ".\n";
	Rain::Time::sleepMs(1000);
	Rain::Networking::Client client;
	client.connect(host);
	std::cout << "Connected client.\n";
	Rain::Time::sleepMs(1000);
	client.send("Hello, world!");
	std::cout << "Sent: \"Hello, world!\".\n";
	Rain::Time::sleepMs(1000);
	client.send("Goodbye!");
	std::cout << "Sent: \"Goodbye!\".\n";
	Rain::Time::sleepMs(1000);
	std::cout << "Stopping server...\n";
	return 0;
}

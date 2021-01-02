#include <rain/networking/client.hpp>
#include <rain/networking/server.hpp>
#include <rain/networking/slave.hpp>

#include <iostream>

class Server : public Rain::Networking::Server<Rain::Networking::Slave> {
	protected:
	void onBeginSlaveTask(Slave &slave) noexcept override {
		std::cout << "Spawned slave " << slave.getNativeSocket()
							<< ". Receiving...\n";
		try {
			char buffer[1024];
			std::size_t recvLen;
			while (true) {
				recvLen = slave.recv(buffer,
					sizeof(buffer),
					Rain::Networking::Socket::RecvFlag::NONE,
					std::chrono::milliseconds(500));
				std::cout << "Received " << recvLen
									<< " bytes: " << std::string(buffer, recvLen) << "\n";
			}
		} catch (const std::exception &e) {
			std::cout << "Receiving terminated by exception:\n" << e.what() << "\n";
		}
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host("localhost", 0), false);
	Rain::Networking::Host host("localhost", server.getService());
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

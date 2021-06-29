/*
Tests basic socket client/server functions from `rain/networking/`.
*/

#include <rain/networking/client.hpp>
#include <rain/networking/server.hpp>
#include <rain/networking/slave.hpp>

#include <iostream>

// Subclass server for custom behavior.
class Server : public Rain::Networking::Server<Rain::Networking::Slave> {
	protected:
	void onBeginSlaveTask(Slave &slave) noexcept override {
		std::cout << "Spawned slave " << slave.getNativeSocket() << ". Receiving..."
							<< std::endl;

		char buffer[32];
		std::size_t recvLen;
		while (true) {
			try {
				recvLen = slave.recv(buffer,
					sizeof(buffer),
					Rain::Networking::Socket::RecvFlag::NONE,
					std::chrono::milliseconds(500));
			} catch (const std::exception &e) {
				std::cout << "Receiving terminated by exception:\n"
									<< e.what() << std::endl;
				break;
			}

			std::cout << "Received " << recvLen << " bytes: \""
								<< std::string(buffer, recvLen) << "\"." << std::endl;
		}
	}
};

int main() {
	Server server;
	server.serve(Rain::Networking::Host(NULL, 0), false);
	Rain::Networking::Host host("localhost", server.getService());
	std::cout << "Started server on port " << host.service.getCStr() << "."
						<< std::endl;

	Rain::Networking::Client client;
	client.connect(host);
	std::cout << "Connected client." << std::endl;
	client.send("Hello, world!");
	std::cout << "Sent: \"Hello, world!\"." << std::endl;
	Rain::Time::sleepMs(600);
	client.send("Goodbye!");
	std::cout << "Sent: \"Goodbye!\"." << std::endl;
	Rain::Time::sleepMs(600);
	std::cout << "Stopping server..." << std::endl;
	return 0;
}

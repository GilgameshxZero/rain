// Test basic Socket Server/Client/Worker interactions from Rain::Networking.
//
// TODO: Cleanup.
#include <rain/literal.hpp>

#include <rain/networking/client.hpp>
#include <rain/networking/server.hpp>
#include <rain/networking/worker.hpp>

#include <cassert>
#include <iostream>

using namespace Rain::Literal;

std::atomic_size_t workerConstructorCalled, workerDestructorCalled;

class Server;
// Subclass Worker which echos all bytes back until client close.
// All derived classes of virtually inherited Socket must call its constructor
// explicitly. The second call to socket passed to Worker does nothing besides
// set addressInfo.
class Worker
		: public Rain::Networking::WorkerInterface<Rain::Networking::Socket> {
	public:
	Worker(
		Rain::Networking::Resolve::AddressInfo const &addressInfo,
		Rain::Networking::Socket &&socket)
			: Interface(addressInfo, std::move(socket)) {
		workerConstructorCalled++;
		std::cout << "Worker constructor called." << std::endl;
	}
	~Worker() {
		workerDestructorCalled++;
		std::cout << "Worker destructor called." << std::endl;
	}

	Worker(Worker const &other) = delete;
	Worker &operator=(Worker const &other) = delete;

	private:
	virtual void onWork() override {
		Rain::Error::consumeThrowable(
			std::function<void()>([this]() {
				// Worker always echos client message.
				std::string buffer(1_zu << 10, '\0');
				std::size_t recvStatus = 0;

				do {
					if (recvStatus != 0) {
						this->send(buffer);
						std::cout << "Worker received: " << buffer;
					}
					recvStatus = this->recv(buffer, {3s});
				} while (recvStatus != 0 && recvStatus != SIZE_MAX);

				if (recvStatus == SIZE_MAX) {
					std::cout << "Worker timed out." << std::endl;
				} else {
					std::cout << "Worker received FIN." << std::endl;
				}

				// Graceful close.
				this->close({3s});
			}),
			RAIN_ERROR_LOCATION)();
	};
};

// Derived clients should declare similar copy/move semantics as the base class.
class Server : public Rain::Networking::
								 ServerInterface<Rain::Networking::Socket, Worker> {
	public:
	Server(
		Rain::Networking::Specification::Specification specification =
			Rain::Networking::Specification::Specification(
				Rain::Networking::Specification::ProtocolFamily::INET6,
				Rain::Networking::Specification::SocketType::STREAM,
				Rain::Networking::Specification::SocketProtocol::TCP),
		std::size_t maxThreads = 1024)
			: Interface(specification, maxThreads) {
		std::cout << "Server constructor called." << std::endl;
	}
	~Server() { std::cout << "Server destructor called." << std::endl; }

	Server(Server const &other) = delete;
	Server &operator=(Server const &other) = delete;

	// workerFactory is exactly the same.
};

// Derived clients should declare similar copy/move semantics as the base class.
class Client
		: public Rain::Networking::ClientInterface<Rain::Networking::Socket> {
	public:
	Client(
		Rain::Networking::Specification::Specification specification =
			Rain::Networking::Specification::Specification(
				Rain::Networking::Specification::ProtocolFamily::INET6,
				Rain::Networking::Specification::SocketType::STREAM,
				Rain::Networking::Specification::SocketProtocol::TCP),
		bool interruptable = true)
			: Interface(specification, interruptable) {
		std::cout << "Client constructor called." << std::endl;
	}
	~Client() { std::cout << "Client destructor called." << std::endl; }

	Client(Client const &other) = delete;
	Client &operator=(Client const &other) = delete;
	Client(Client &&other) : Interface(std::move(other)) {}
};

int main() {
	// Send test message with client to Server and expect it back.
	// Both sides close gracefully.
	std::cout << std::endl;
	auto start = std::chrono::steady_clock::now();

	{
		Server Server;
		Server.serve();
		Rain::Networking::Host host(Server.getTargetHost());
		std::cout << "Server bound to " << host << "." << std::endl;

		Client client;
		client.connect(Rain::Networking::Host("", host.service));

		// Client sends.
		client.send("Hello from the client!\n");
		client.shutdown();

		// Worker may have destructed by now, after receiving shutdown from client.

		// Client receives.
		std::string buffer(1_zu << 10, '\0');
		std::size_t recvTotal = 0, recvStatus = 0;

		// !!! WARNING: recv should ALWAYS be wrapped in try/catch in case the other
		// end aborts connection. Here, Worker MUST gracefully close to allow this
		// code not to throw.
		do {
			recvTotal += recvStatus;
			recvStatus = client.recv(&buffer[recvTotal], buffer.size() - recvTotal);
		} while (recvStatus != 0 && recvStatus != SIZE_MAX);
		buffer.resize(recvTotal);
		std::cout << "Client received: " << buffer;
		assert(buffer == "Hello from the client!\n");

		// Both close gracefully.
		client.close();
		Server.close();
	}

	assert(workerConstructorCalled == 1);
	assert(workerDestructorCalled == 1);
	assert(std::chrono::steady_clock::now() - start < 1s);

	// Client aborts connection. Server also aborts.
	// Server should handle this, not crash, and ideally catch within Worker
	// instead of propagating to Server.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Server Server;
		Server.serve();
		Rain::Networking::Host host(Server.getTargetHost());
		std::cout << "Server bound to " << host << "." << std::endl;

		{
			Client client;
			client.connect(Rain::Networking::Host("", host.service));
			client.send("Client is about to abort.\n");

			// Wait for Worker to process.
			std::this_thread::sleep_for(1s);

			// Client aborts.
		}

		// Server aborts.
		// Worker gets aborted connection from client abort, must handle.
	}

	assert(workerConstructorCalled == 2);
	assert(workerDestructorCalled == 2);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Server gracefully closes while client sending, then client gracefully
	// closes.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Client client;

		{
			Server Server;
			Server.serve();
			Rain::Networking::Host host(Server.getTargetHost());
			std::cout << "Server bound to " << host << "." << std::endl;

			// Parallel connect may spawn multiple Workers, so keep serial.
			client.connect(Rain::Networking::Host("localhost", host.service), false);
			client.send("Server is about to close.\n");

			// Wait for Worker to process.
			std::this_thread::sleep_for(1s);

			Server.close();
		}

		client.close();
	}

	assert(workerConstructorCalled == 3);
	assert(workerDestructorCalled == 3);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Server aborts, then client closes gracefully.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Client client;

		{
			Server Server;
			Server.serve();
			Rain::Networking::Host host(Server.getTargetHost());
			std::cout << "Server bound to " << host << "." << std::endl;

			client.connect(Rain::Networking::Host("localhost", host.service), false);
			client.send("Server is about to abort.\n");

			// Wait for Worker to process.
			std::this_thread::sleep_for(1s);

			// Server aborts.
		}

		client.close();
	}

	assert(workerConstructorCalled == 4);
	assert(workerDestructorCalled == 4);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Server aborts, then client aborts.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Client client;

		{
			Server Server;
			Server.serve();
			Rain::Networking::Host host(Server.getTargetHost());
			std::cout << "Server bound to " << host << "." << std::endl;

			client.connect({"localhost", host.service}, false);
			client.send("Server is about to abort.\n");

			// Wait for Worker to process.
			std::this_thread::sleep_for(1s);

			// Server aborts.
		}

		// Client aborts.
	}

	assert(workerConstructorCalled == 5);
	assert(workerDestructorCalled == 5);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Client aborts, then server closes.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Server Server;
		Server.serve();
		Rain::Networking::Host host(Server.getTargetHost());
		std::cout << "Server bound to " << host << "." << std::endl;

		{
			Client client;
			client.connect({"", host.service}, false);
			client.send("Client is about to abort.\n");

			// Wait for Worker to process.
			std::this_thread::sleep_for(1s);

			// Client aborts.
		}

		// Server closes.
		Server.close();
	}

	assert(workerConstructorCalled == 6);
	assert(workerDestructorCalled == 6);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Correct use of Server/Client: any calls to serve/connect should be matched
	// with close, and any calls to recv should be wrapped in try/catch.
	//
	// Guarantees NBTA contract for all parties. NBTA-ness for Clients or for
	// Socket/Worker pairs are independant.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Server Server;
		Server.serve();
		Rain::Networking::Host host(Server.getTargetHost());
		std::cout << "Server bound to " << host << "." << std::endl;

		Client client;
		client.connect({"", host.service}, false);
		client.send("Hello from the client!\n");
		client.shutdown();

		// Wrap recv in try/catch in case other end aborts.
		Rain::Error::consumeThrowable(
			std::function<void()>([&client]() {
				std::string buffer(1_zu << 10, '\0');
				std::size_t recvTotal = 0, recvStatus = 0;

				do {
					recvTotal += recvStatus;
					recvStatus =
						client.recv(&buffer[recvTotal], buffer.size() - recvTotal);
				} while (recvStatus != 0 && recvStatus != SIZE_MAX);
				buffer.resize(recvTotal);
				std::cout << "Client received: " << buffer;
				assert(buffer == "Hello from the client!\n");
			}),
			RAIN_ERROR_LOCATION);

		// Both close gracefully, in any order, if serve/connect was called
		// previously. Server operations should only ever be called from a single
		// thread.
		client.close();
		Server.close();
	}

	assert(workerConstructorCalled == 7);
	assert(workerDestructorCalled == 7);
	assert(std::chrono::steady_clock::now() - start < 2s);

	// Client times out then aborts, Worker cleans up after timeout, Server
	// aborts.
	std::cout << std::endl;
	start = std::chrono::steady_clock::now();

	{
		Server Server;
		Server.serve();
		Rain::Networking::Host host(Server.getTargetHost());
		std::cout << "Server bound to " << host << "." << std::endl;

		Client client;
		client.connect(Rain::Networking::Host("localhost", host.service));
		client.send("Client will trigger timeout soon.\n");

		// Times out waiting Worker: 3s for recv, 3s for close (waiting on FIN from
		// client).
		std::this_thread::sleep_for(7s);
		// Not == 8 since connect was parallel.
		assert(workerConstructorCalled > 7);
		assert(workerDestructorCalled > 7);

		// Server closes and client aborts.
		Server.close();
	}

	assert(std::chrono::steady_clock::now() - start < 8s);

	return 0;
}

// Test basic Socket Server/Client/Worker interactions from Rain::Networking.
#include <rain/literal.hpp>
#include <rain/networking/server.hpp>
#include <rain/networking/socket-options.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	class MyWorker : public Worker<
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption> {
		virtual void onWork() override {
			// Worker always echos client message.
			std::string buffer(1_zu << 2, '\0');
			std::size_t recvStatus{0};
			Rain::Time::Timeout timeout;

			// In case peer resets during recv.
			Rain::Error::consumeThrowable(
				[this, &buffer, &recvStatus, &timeout]() {
					do {
						if (recvStatus != 0) {
							this->send(buffer);
							std::cout << "MyWorker received: " << buffer << std::endl;
						}
						timeout = Rain::Time::Timeout(3s);
					} while ((recvStatus = this->recv(buffer, timeout)));
				},
				RAIN_ERROR_LOCATION)();

			if (timeout.isPassed()) {
				std::cout << "MyWorker timed out." << std::endl;
			} else {
				std::cout << "MyWorker received FIN or interrupt." << std::endl;
			}

			// Graceful shutdown.
			Rain::Error::consumeThrowable(
				[this]() { this->shutdown(ShutdownOpt::GRACEFUL, 3s); })();
		}

		std::size_t &workersDestructed;

		public:
		MyWorker(
			NativeSocket nativeSocket,
			SocketInterface *interrupter,
			std::size_t &workersDestructed)
				: Worker(nativeSocket, interrupter),
					workersDestructed(workersDestructed) {
			std::cout << __func__ << std::endl;
		}
		virtual ~MyWorker() {
			std::cout << __func__ << std::endl;
			this->workersDestructed++;
		}
	};

	class MyServer : public Server<
										 MyWorker,
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption,
										 Ipv6OnlySocketOption> {
		using Server::Server;

		class _MyServer {
			public:
			_MyServer(MyServer *) { std::cout << __func__ << std::endl; }
		};
		_MyServer _myServer = _MyServer(this);

		std::size_t _workersDestructed = 0;

		virtual MyWorker makeWorker(
			NativeSocket nativeSocket,
			SocketInterface *interrupter) override {
			return {nativeSocket, interrupter, this->_workersDestructed};
		}

		public:
		virtual ~MyServer() {
			std::cout << __func__ << std::endl;
			this->destruct();
		}
		std::size_t workersDestructed() { return this->_workersDestructed; }
	};

	class MyClient : public Client<
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption> {
		using Client::Client;

		class _MyClient {
			public:
			_MyClient(MyClient *) { std::cout << __func__ << std::endl; }
		};
		_MyClient _myServer = _MyClient(this);

		public:
		virtual ~MyClient() { std::cout << __func__ << std::endl; }
	};

	// Connect to server with IPv6 on both sides.
	{
		MyServer server(":0");
		MyClient client(Host{"", server.host().service});

		std::vector<NamedSocketSpecInterface *> sockets{&server, &client};
		for (auto socket : sockets) {
			std::cout << "Socket host: " << socket->host() << std::endl;
		}
		std::cout << "Client peer: " << client.peerHost() << std::endl;
	}

	// Connect to IPv6 only server with IPv4 client fails without waiting for
	// entire timeout.
	{
		std::cout << std::endl;
		class MyClient : public Client<
											 Ipv4FamilyInterface,
											 StreamTypeInterface,
											 TcpProtocolInterface,
											 NoLingerSocketOption> {
			using Client::Client;

			class _MyClient {
				public:
				_MyClient(MyClient *) { std::cout << __func__ << std::endl; }
			};
			_MyClient _myServer = _MyClient(this);

			public:
			~MyClient() { std::cout << __func__ << std::endl; }
		};

		MyServer server(":0");

		try {
			MyClient client(Host{"", server.host().service}, {});
			throw std::runtime_error(
				"Should have thrown exception during client constructor.\n");
		} catch (Exception const &exception) {
			std::cout << exception.what();
		}
	}

	// Connecting with IPv4 Client to IPv6 Server on Dual-Stack works.
	{
		std::cout << std::endl;
		class MyServer : public Server<
											 MyWorker,
											 Ipv6FamilyInterface,
											 StreamTypeInterface,
											 TcpProtocolInterface,
											 NoLingerSocketOption,
											 DualStackSocketOption> {
			using Server::Server;

			class _MyServer {
				public:
				_MyServer(MyServer *) { std::cout << __func__ << std::endl; }
			};
			_MyServer _myServer = _MyServer(this);

			std::size_t workersDestructed{0};

			virtual MyWorker makeWorker(
				NativeSocket nativeSocket,
				SocketInterface *interrupter) override {
				return {nativeSocket, interrupter, this->workersDestructed};
			}

			public:
			~MyServer() {
				std::cout << __func__ << std::endl;
				this->destruct();
			}
		};

		class MyClient : public Client<
											 Ipv4FamilyInterface,
											 StreamTypeInterface,
											 TcpProtocolInterface,
											 NoLingerSocketOption> {
			using Client::Client;

			class _MyClient {
				public:
				_MyClient(MyClient *) {
					std::cout << "MyClient constructor." << std::endl;
				}
			};
			_MyClient _myServer = _MyClient(this);

			public:
			~MyClient() { std::cout << "MyClient destructor." << std::endl; }
		};

		MyServer server(":0");
		MyClient client(Host{"", server.host().service}, {});
	}

	// MyWorker correctly echos messages back.
	{
		std::cout << std::endl;
		MyServer server(":0");
		{
			MyClient client(Host{"", server.host().service}, {});
			client.send("Hello from the client!");

			// This should not throw, since Worker should always be open longer than
			// the client.
			client.shutdown();
		}
	}

	// Closing the server will interrupt all Workers regardless of what the
	// Client is up to.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		std::cout << std::endl;
		std::unique_ptr<MyServer> server(new MyServer(":0"));
		std::unique_ptr<MyClient> client(
			new MyClient(Host{"", server->host().service}, {}));
		server.reset();
		client.reset();
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;
		assert(timeElapsed < 1s);
	}

	// Client times out in 3 seconds and takes 3 more seconds to shutdown.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		std::cout << std::endl;
		MyServer server(":0");
		MyClient client(Host{"", server.host().service}, {});
		std::this_thread::sleep_for(7s);
		std::cout << "Server workers: " << server.workers()
							<< ", threads: " << server.threads() << std::endl;
		assert(server.workers() == 0);
		assert(server.threads() == 2);
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;
		assert(timeElapsed < 8s);
	}

	// Workers destructed counter is incremented correctly.
	{
		std::cout << std::endl;
		MyServer server(":0");
		{
			MyClient client(Host{"", server.host().service});
			MyClient client2(client.peerName());
			MyClient client3(client.peerName());

			// Wait a bit so that worker construction can finish (so that setting no
			// linger will not error if the socket is already disconnected).
			std::this_thread::sleep_for(1s);
		}
		// Wait a bit so that workers can destruct.
		std::this_thread::sleep_for(1s);
		assert(server.workersDestructed() == 3);
	}

	return 0;
}

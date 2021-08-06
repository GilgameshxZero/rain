// Tests Networking::Socket for basic capabilities (bind, listen, accept, recv,
// interrupt).
#include <rain/literal.hpp>

#include <rain/networking/socket.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// Interrupt accepting socket immediately.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		Socket server;
		server.bind();
		server.listen();

		std::thread acceptThread([&server]() {
			std::cout << "Accepting..." << std::endl;
			server.accept();
		});
		std::cout << "Interrupting..." << std::endl;
		server.interrupt();
		acceptThread.join();

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed < 0.5s);
	}

	// Interrupt a client while it is receiving should stop both it and the worker
	// socket.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		Socket server;
		server.bind();
		server.listen();

		std::thread acceptThread([&server]() {
			std::cout << "Accepting..." << std::endl;
			auto acceptRes = server.accept();
			std::cout << "Worker recving..." << std::endl;

			std::string buffer(128, '\0');
			// When the client gets interrupt, it no longer is able to close
			// gracefully, and may abort. That will cause our recv to throw.
			Rain::Error::consumeThrowable(
				[&buffer, &acceptRes]() { acceptRes.first.recv(buffer); },
				RAIN_ERROR_LOCATION);
			acceptRes.first.close();
		});

		Socket client;
		client.connect({"", server.getTargetHost().service});
		std::thread clientThread([&client]() {
			std::cout << "Client recving..." << std::endl;
			std::string buffer(128, '\0');
			client.recv(buffer);
			client.close();
		});

		std::cout << "Sleeping for 1s..." << std::endl;
		std::this_thread::sleep_for(1s);
		std::cout << "Interrupting client..." << std::endl;
		client.interrupt();
		clientThread.join();
		acceptThread.join();

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << "." << std::endl
							<< std::endl;
		assert(timeElapsed < 1.5s);
	}

	// An interrupt should cause all calls dependent on poll to timeout, and
	// should also interrupt all future poll calls, causing them to timeout
	// immediately.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		Socket server;
		server.bind();
		server.listen();

		std::thread acceptThread([&server]() {
			std::cout << "Accepting..." << std::endl;
			auto acceptRes = server.accept();
			std::cout << "Worker recving..." << std::endl;

			std::string buffer(128, '\0');
			// When the client gets interrupt, it no longer is able to close
			// gracefully, and may abort. That will cause our recv to throw.
			Rain::Error::consumeThrowable(
				[&buffer, &acceptRes]() { acceptRes.first.recv(buffer); },
				RAIN_ERROR_LOCATION);
			acceptRes.first.close();
		});

		Socket client;
		client.connect({"", server.getTargetHost().service});
		std::thread clientThread([&client]() {
			std::cout << "Client recving..." << std::endl;
			std::string buffer(128, '\0');
			client.recv(buffer);
			std::cout << "Client recving again..." << std::endl;
			client.recv(buffer);
			client.close();
		});

		std::cout << "Sleeping for 1s..." << std::endl;
		std::this_thread::sleep_for(1s);
		std::cout << "Interrupting client..." << std::endl;
		client.interrupt();
		clientThread.join();
		acceptThread.join();

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << "." << std::endl
							<< std::endl;
		assert(timeElapsed < 1.5s);
	}
	return 0;
}

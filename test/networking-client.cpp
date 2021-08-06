// Tests Networking::Client, as well as the implementation of recv.
#include <rain/literal.hpp>

#include <rain/networking/client.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	{
		auto timeBegin = std::chrono::steady_clock::now();
		Client client;
		client.connect({"google.com:80"});

		// Send HTTP/0.9 request.
		client.send("GET /\r\n");

		// google.com should close gracefully. We should expect the first few
		// characters as "HTTP/1.0 200 OK" (google.com doesn't send 0.9 responses).
		std::size_t recvStatus = 0;
		std::string buffer(1_zu << 4, '\0'), response;

		recvStatus = client.recv(buffer);
		std::size_t totalBytes = 0;
		while (recvStatus != 0 && recvStatus != SIZE_MAX) {
			response += buffer;
			if (totalBytes < (1_zu << 10)) {
				std::cout << buffer;
			}
			totalBytes += buffer.size();
			recvStatus = client.recv(buffer);
		}

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << "." << std::endl
							<< std::endl;
		assert(timeElapsed < 5s);
		assert(response.substr(0, 15) == "HTTP/1.0 200 OK");
	}

	// Sleep on gai threads for valgrind.
	std::this_thread::sleep_for(1s);

	return 0;
}

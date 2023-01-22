// Tests Networking::Client, as well as the implementation of send/recv.
#include <rain/literal.hpp>
#include <rain/networking/client.hpp>
#include <rain/networking/socket-options.hpp>
#include <rain/time/time.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	{
		auto timeBegin = std::chrono::steady_clock::now();
		// ConnectedSocket, NamedSocket are noops, but this is good practice in case
		// they aren't.
		Client<
			Ipv4FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			NoLingerSocketOption>
			client("google.com:80");
		std::cout << "Connected to " << client.peerHost() << std::endl;
		// HTTP 0.9.
		client.send("GET /\r\n");

		std::string buffer(1_zu << 10, '\0');
		client.recv(buffer);
		std::cout << buffer << std::endl;
		assert(buffer.substr(0, 15) == "HTTP/1.0 200 OK");

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << "." << std::endl
							<< std::endl;
		assert(timeElapsed < 5s);
	}

	return 0;
}

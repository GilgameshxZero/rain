// Tests Networking::Tcp, and in particular the iostream capability.
#include <rain/networking/tcp.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;

	{
		Rain::Networking::Tcp::Client client;
		client.connect({"google.com:80"});
		client << "GET / HTTP/1.1\r\n"
					 << "Host: google.com\r\n"
					 << "\r\n";
		client.flush();

		auto timeBegin = std::chrono::steady_clock::now();
		std::string line, response;
		while (std::getline(client, line)) {
			response += line;
			std::cout << line << std::endl;
		}
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << "." << std::endl
							<< std::endl;

		assert(timeElapsed > 60s && timeElapsed < 65s);
		assert(response.substr(0, 30) == "HTTP/1.1 301 Moved Permanently");

		// Should be 60s (default) until getline/recv times out.
		std::cout << "EOFBIT: " << client.eof() << std::endl
							<< "FAILBIT: " << client.fail() << std::endl
							<< "BADBIT: " << client.bad() << std::endl
							<< "GOODBIT: " << client.good() << std::endl;
	}

	return 0;
}

// Tests Networking::Tcp, and in particular the iostream capability.
#include <rain.hpp>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	Tcp::Client<
		1_zu << 10,
		1_zu << 10,
		3000,
		3000,
		Ipv4FamilyInterface,
		StreamTypeInterface,
		TcpProtocolInterface,
		NoLingerSocketOption>
		client("google.com:80");
	std::cout << "Connected to " << client.peerHost() << std::endl;
	client << "GET / HTTP/1.1\r\n"
				 << "Host: google.com\r\n\r\n"
				 << std::flush;

	// Streaming an rdbuf will never set eof even if things go bad.
	std::stringstream ss;
	ss << client.rdbuf();
	std::cout << ss.str().substr(0, 1_zu << 10) << std::endl;
	assert(ss.str().substr(0, 30) == "HTTP/1.1 301 Moved Permanently");
	std::cout << "EOFBIT: " << client.eof() << std::endl
						<< "FAILBIT: " << client.fail() << std::endl
						<< "BADBIT: " << client.bad() << std::endl
						<< "GOODBIT: " << client.good() << std::endl
						<< std::endl;
	assert(client.good());

	// Another read-in will error.
	std::string tmp;
	client >> tmp;
	std::cout << "EOFBIT: " << client.eof() << std::endl
						<< "FAILBIT: " << client.fail() << std::endl
						<< "BADBIT: " << client.bad() << std::endl
						<< "GOODBIT: " << client.good() << std::endl
						<< std::endl;
	assert(!client.good());

	// And further reads will not even take the entire timeout.
	auto timeBegin = std::chrono::steady_clock::now();
	client >> tmp;
	std::cout << "EOFBIT: " << client.eof() << std::endl
						<< "FAILBIT: " << client.fail() << std::endl
						<< "BADBIT: " << client.bad() << std::endl
						<< "GOODBIT: " << client.good() << std::endl;
	assert(!client.good());
	auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
	std::cout << "Time elapsed: " << timeElapsed << std::endl;
	assert(timeElapsed < 1s);

	return 0;
}

// Tests for Networking::Host.
#include <rain/networking/host.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// Empty constructor and implicit conversions work as expected.
	{
		Host host;

		assert(host.node == "");
		assert(host.node == ""s);

		assert(host.service == "");
		assert(host.service == ""s);

		assert(static_cast<std::string>(host) == ":");
	}

	// Host:Service constructor.
	{
		Host host("google.com:80");

		assert(host.node == "google.com");
		assert(host.service == "80");

		assert(static_cast<std::string>(host) == "google.com:80");
	}
	{
		Host host("google.com:");

		assert(host.node == "google.com");
		assert(host.service == "");
		assert(static_cast<std::string>(host) == "google.com");
	}
	{
		Host host("google.com");

		assert(host.node == "google.com");
		assert(host.service == "");
		assert(static_cast<std::string>(host) == "google.com");
	}
	{
		Host host(":80");

		assert(host.node == "");
		assert(host.service == "80");
		assert(static_cast<std::string>(host) == ":80");
	}

	// std::stringstream output.
	{
		Host host("google.com:80");
		std::stringstream stream;
		stream << host;

		assert(stream.str() == "google.com:80");
	}

	return 0;
}

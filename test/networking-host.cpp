// Tests for Networking::Host.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// Empty constructor and implicit conversions work as
	// expected.
	{
		Host host;

		releaseAssert(host.node == "");
		releaseAssert(host.node == ""s);

		releaseAssert(host.service == "");
		releaseAssert(host.service == ""s);

		releaseAssert(static_cast<std::string>(host) == ":");
	}

	// Host:Service constructor.
	{
		Host host("google.com:80");

		releaseAssert(host.node == "google.com");
		releaseAssert(host.service == "80");

		releaseAssert(
			static_cast<std::string>(host) == "google.com:80");
	}
	{
		Host host("google.com:");

		releaseAssert(host.node == "google.com");
		releaseAssert(host.service == "");
		releaseAssert(
			static_cast<std::string>(host) == "google.com");
	}
	{
		Host host("google.com");

		releaseAssert(host.node == "google.com");
		releaseAssert(host.service == "");
		releaseAssert(
			static_cast<std::string>(host) == "google.com");
	}
	{
		Host host(":80");

		releaseAssert(host.node == "");
		releaseAssert(host.service == "80");
		releaseAssert(static_cast<std::string>(host) == ":80");
	}

	// std::stringstream output.
	{
		Host host("google.com:80");
		std::stringstream stream;
		stream << host;

		releaseAssert(stream.str() == "google.com:80");
	}

	// Copy/move semantics.
	{
		Host host1("google.com:80");

		// Copy.
		Host host2(host1);

		releaseAssert(host1 == host2);

		// Move.
		Host host3(std::move(host1));

		releaseAssert(host1 != host2);
		releaseAssert(host3 == host2);
	}

	return 0;
}

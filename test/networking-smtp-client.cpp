// Tests for Networking::Smtp::Client. These tests require outgoing IPv4 port 25
// to be unblocked by firewall.
#include <rain/networking/smtp.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <system_error>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Smtp;

	// A simple sequence to gmail.com SMTP using IPv4.
	{
		// Invoke with Smtp::Client constructor argument defaults to prove they are
		// accessible.
		Client client(true, Rain::Networking::Specification::ProtocolFamily::INET);
		if (client.connectMx({"gmail.com:25"})) {
			throw std::runtime_error("Failed to connect to MX SMTP server.\n");
		}
		{
			std::cout << "Connected to " << client.getTargetHost() << "."
								<< std::endl;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::SERVICE_READY);
		}
		{
			Request req(Command::HELO, "domain.name.blocked.for.privacy");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
		}
		{
			Request req(Command::MAIL, "FROM:<from@domain.name.blocked.for.privacy>");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
		}

		// Multiple FROM commands overwrite the previous one.
		{
			Request req(
				Command::MAIL, "FROM:<from-2@domain.name.blocked.for.privacy>");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
		}

		{
			Request req(Command::RCPT, "TO:<test@gmail.com>");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(
				res.statusCode ==
				StatusCode::REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT);
		}
		{
			Request req(Command::VRFY, "<test@gmail.com>");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::CANNOT_VERIFY);
		}
		{
			Request req(Command::NOOP);
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
		}
		{
			Request req(Command::HELP);
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::HELP_MESSAGE);
		}
		{
			Request req(Command::QUIT);
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::SERVICE_CLOSING);
		}

		client.close();
	}

	// Querying EHLO of gmail.com SMTP.
	{
		Client client(true, Rain::Networking::Specification::ProtocolFamily::INET);
		if (client.connectMx({"gmail.com:25"})) {
			throw std::runtime_error("Failed to connect to MX SMTP server.\n");
		}

		{
			std::cout << "Connected to " << client.getTargetHost() << "."
								<< std::endl;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::SERVICE_READY);
		}

		{
			Request req(Command::EHLO, "domain.name.blocked.for.privacy");
			std::cout << req;
			client << req;
			Response res = client.recvResponse();
			std::cout << res;
			assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
		}
	}

	std::this_thread::sleep_for(1s);

	return 0;
}

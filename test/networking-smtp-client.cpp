// Tests for Networking::Smtp::Client. These tests require outgoing IPv4 port 25
// to be unblocked by firewall.
#include <rain/networking/smtp/client.hpp>
#include <rain/networking/smtp/command.hpp>
#include <rain/networking/smtp/status-code.hpp>
#include <rain/networking/socket-options.hpp>
#include <rain/networking/specification.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <system_error>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	try {
		Smtp::Client<
			Smtp::Request,
			Smtp::Response,
			1_zu << 10,
			1_zu << 10,
			15000,
			15000,
			Ipv4FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			NoLingerSocketOption>
			client(getMxRecords("gmail.com"));
	} catch (Rain::Networking::Exception const &exception) {
		if (exception.getError() == Rain::Networking::Error::TIMED_OUT) {
			// Likely SMTP is blocked by ISP; skip test.
			std::cout << "SMTP blocked (by ISP). Skipping test...\n";
			return 0;
		}

		throw;
	}

	// A simple sequence to gmail.com SMTP using IPv4.
	{
		Smtp::Client<
			Smtp::Request,
			Smtp::Response,
			1_zu << 10,
			1_zu << 10,
			15000,
			15000,
			Ipv4FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			NoLingerSocketOption>
			client(getMxRecords("gmail.com"));

		{
			std::cout << "Connected to " << client.peerHost() << "." << std::endl;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::SERVICE_READY);
		}
		{
			Smtp::Request req(Smtp::Command::HELO, "domain.name.blocked.for.privacy");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
		}
		{
			Smtp::Request req(
				Smtp::Command::MAIL, "FROM:<from@domain.name.blocked.for.privacy>");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
		}

		// Multiple FROM commands overwrite the previous one.
		{
			Smtp::Request req(
				Smtp::Command::MAIL, "FROM:<from-2@domain.name.blocked.for.privacy>");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
		}

		{
			Smtp::Request req(Smtp::Command::RCPT, "TO:<test@gmail.com>");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(
				res.statusCode ==
				Smtp::StatusCode::REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT);
		}
		{
			Smtp::Request req(Smtp::Command::VRFY, "<test@gmail.com>");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::CANNOT_VERIFY);
		}
		{
			Smtp::Request req(Smtp::Command::NOOP);
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
		}
		{
			Smtp::Request req(Smtp::Command::HELP);
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::HELP_MESSAGE);
		}
		{
			Smtp::Request req(Smtp::Command::QUIT);
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::SERVICE_CLOSING);
		}

		client.shutdown();
	}

	// Querying EHLO of gmail.com SMTP.
	{
		Smtp::Client<
			Smtp::Request,
			Smtp::Response,
			1_zu << 10,
			1_zu << 10,
			15000,
			15000,
			Ipv4FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			NoLingerSocketOption>
			client(getMxRecords("gmail.com"));

		{
			std::cout << "Connected to " << client.peerHost() << "." << std::endl;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::SERVICE_READY);
		}

		{
			Smtp::Request req(Smtp::Command::EHLO, "domain.name.blocked.for.privacy");
			std::cout << req;
			client << req;
			auto res = client.recv();
			std::cout << res;
			assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
		}
	}

	return 0;
}

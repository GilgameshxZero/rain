// Tests for Networking::Smtp::Server.
#include <rain/networking/smtp.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <system_error>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Smtp;

	{
		Server server;
		server.serve();

		std::cout << "Serving on " << server.getTargetHost() << "..." << std::endl;

		{
			Client client(
				true, Rain::Networking::Specification::ProtocolFamily::INET);
			if (client.connect({"127.0.0.1", server.getTargetHost().service})) {
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
				Request req(Command::NOOP);
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
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
				Request req(
					Command::MAIL, "FROM:<from@domain.name.blocked.for.privacy>");
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
			}

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
				Request req(Command::RCPT, "TO:<test@domain.name>");
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
			}

			{
				Request req(Command::RCPT, "TO:<test-2@domain.name>");
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::REQUEST_COMPLETED);
			}

			{
				Request req(Command::DATA);
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::START_MAIL_INPUT);

				// Send some data as email.
				std::string data(
					"From: from-2@domain.name.blocked.for.privacy\r\nTo: "
					"test-2@domain.name\r\nSubject: Hi!\r\n\r\nSome message "
					"here.\r\n\r\n.\r\n");
				std::cout << data;
				client << data;
				client.flush();
				Response dataRes = client.recvResponse();
				std::cout << dataRes;
				assert(
					dataRes.statusCode ==
					StatusCode::TRANSACTION_FAILED);
			}

			{
				Request req(Command::VRFY, "<test@domain.name>");
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::CANNOT_VERIFY);
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
				Request req(Command::TURN);
				std::cout << req;
				client << req;
				Response res = client.recvResponse();
				std::cout << res;
				assert(res.statusCode == StatusCode::COMMAND_NOT_IMPLEMENTED);
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

		// Let server remain open for 60s for playtest.
		std::cout << "Waiting 60s..." << std::endl;
		std::this_thread::sleep_for(60s);
		server.close();
	}

	return 0;
}

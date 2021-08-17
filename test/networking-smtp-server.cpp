// Tests for Networking::Smtp::Server.
#include <rain/networking/smtp.hpp>
#include <rain/networking/socket-options.hpp>
#include <rain/networking/specification.hpp>

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <system_error>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// Custom Request/Response output to cout when received via pp-chain.
	class MyRequest : public Smtp::Request {
		public:
		using Request::Request;

		virtual void recvWith(std::istream &stream) override {
			Request::recvWith(stream);
			std::cout << "Request:\n" << *this << std::endl;
		}
	};
	class MyResponse : public Smtp::Response {
		public:
		using Response::Response;

		virtual void recvWith(std::istream &stream) override {
			Response::recvWith(stream);
			std::cout << "Response:\n" << *this << std::endl;
		}
	};

	class MyWorker : public Smtp::Worker<
										 MyRequest,
										 Smtp::Response,
										 1_zu << 10,
										 1_zu << 10,
										 5000,
										 5000,
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption> {
		using Worker::Worker;
	};

	class MyServer : public Smtp::Server<
										 MyWorker,
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 DualStackSocketOption,
										 NoLingerSocketOption> {
		using Server::Server;

		private:
		virtual MyWorker makeWorker(
			NativeSocket nativeSocket,
			SocketInterface *interrupter) override {
			return {nativeSocket, interrupter};
		}

		public:
		~MyServer() { this->destruct(); }
	};

	class MyClient : public Smtp::Client<
										 Smtp::Request,
										 MyResponse,
										 1_zu << 10,
										 1_zu << 10,
										 15000,
										 15000,
										 Ipv4FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption> {
		using Client::Client;
	};

	{
		MyServer server(":0");
		std::cout << "Serving on " << server.host() << std::endl;

		{
			MyClient client(Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;

			{
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::SERVICE_READY);
			}

			{
				client.send({Smtp::Command::NOOP, ""});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::HELO, "domain.name"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::MAIL, "FROM:<from@domain.name>"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::MAIL, "FROM:<from-2@domain.name>"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::RCPT, "TO:<to@domain.name>"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::RCPT, "TO:<to-2@domain.name>"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::REQUEST_COMPLETED);
			}

			{
				client.send({Smtp::Command::DATA, ""});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::START_MAIL_INPUT);

				std::string data(
					"From: from-2@domain.name.blocked.for.privacy\r\nTo: "
					"test-2@domain.name\r\nSubject: Hi!\r\n\r\nSome message "
					"here.\r\n\r\n.\r\n");
				std::cout << data;
				client << data << std::flush;
				auto dataRes = client.recv();
				assert(dataRes.statusCode == Smtp::StatusCode::TRANSACTION_FAILED);
			}

			{
				client.send({Smtp::Command::VRFY, "TO:<to-2@domain.name>"});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::CANNOT_VERIFY);
			}

			{
				client.send({Smtp::Command::HELP, ""});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::HELP_MESSAGE);
			}

			{
				client.send({Smtp::Command::TURN, ""});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::COMMAND_NOT_IMPLEMENTED);
			}

			{
				client.send({Smtp::Command::QUIT, ""});
				auto res = client.recv();
				assert(res.statusCode == Smtp::StatusCode::SERVICE_CLOSING);
			}

			Rain::Error::consumeThrowable([&client]() { client.shutdown(); });
		}

		// Let server remain open for 60s for playtest.
		std::cout << "Serving on " << server.host() << std::endl;
		std::this_thread::sleep_for(60s);
	}

	return 0;
}

// Tests Networking::Http::Server.
#include <rain/networking/http/client.hpp>
#include <rain/networking/http/server.hpp>
#include <rain/networking/http/worker.hpp>
#include <rain/networking/socket-options.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// Custom Request/Response output to cout when received via pp-chain.
	class MyRequest : public Http::Request {
		public:
		using Request::Request;

		virtual void recvWith(std::istream &stream) override {
			Request::recvWith(stream);
			std::cout << "Request without body:\n"
								<< this->method << ' ' << this->target << ' ' << this->version
								<< '\n'
								<< this->headers << std::endl;
		}
	};
	class MyResponse : public Http::Response {
		public:
		using Response::Response;

		virtual void recvWith(std::istream &stream) override {
			Response::recvWith(stream);
			std::cout << "Response without body:\n"
								<< this->version << ' ' << this->statusCode << ' '
								<< this->reasonPhrase << '\n'
								<< this->headers << std::endl;
		}
	};

	class MyWorker : public Http::Worker<
										 MyRequest,
										 Http::Response,
										 1_zu << 10,
										 1_zu << 10,
										 5000,
										 5000,
										 Ipv6FamilyInterface,
										 StreamTypeInterface,
										 TcpProtocolInterface,
										 NoLingerSocketOption> {
		using Worker::Worker;

		private:
		ResponseAction reqSimple(Request &, std::smatch const &) {
			return {{StatusCode::OK}};
		}
		ResponseAction reqEcho(Request &req, std::smatch const &) {
			// Must read full body into memory, since req.body is of indeterminate
			// length.
			std::stringstream stream;
			stream << req.body;

			return {
				{StatusCode::OK,
				 {{{"Your-Method", req.method}, {"Response-Test-Header", "test"}}},
				 stream.str(),
				 "reason"}};
		}
		ResponseAction reqPlay(Request &req, std::smatch const &) {
			return {
				{StatusCode::OK,
				 {{{"Content-Type", "text/plain; charset=UTF-8"}}},
				 "Your Method: "s + static_cast<std::string>(req.method) +
					 "\nHello world!"s,
				 "bruh"}};
		}

		virtual std::vector<RequestFilter> filters() override {
			return {
				{".*", "/simple/?", {Method::GET}, &MyWorker::reqSimple},
				{".*", "/echo/?", {Method::GET, Method::POST}, &MyWorker::reqEcho},
				{".*", "/play.*", {Method::GET, Method::POST}, &MyWorker::reqPlay}};
		}

		// Override send pp-chain to add server signature.
		virtual void send(Http::Response &res) override {
			res.headers.server("MyServer");
			Worker::send(res);
		}
	};

	class MyServer : public Http::Server<
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

	class MyClient : public Http::Client<
										 Http::Request,
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

		// Basic request to /simple.
		{
			MyClient client({"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;
			client.send({Http::Method::GET, "/simple"s});
			auto res = client.recv();
			assert(res.version == Http::Version::_1_1);
			assert(res.statusCode == Http::StatusCode::OK);
			assert(res.headers.contentLength() == 0);
			assert(res.headers.server() == "MyServer");
			client.shutdown();
		}

		// Basic 404ing request.
		{
			MyClient client({"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;
			client.send({Http::Method::GET, "/does-not-exist"s});
			auto res = client.recv();
			assert(res.version == Http::Version::_1_1);
			assert(res.statusCode == Http::StatusCode::NOT_FOUND);
			assert(res.headers.contentLength() == 0);
			assert(res.headers.server() == "MyServer");
			client.shutdown();
		}

		// Server and client ignore 0.9 headers and server ignores body.
		{
			MyClient client({"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;
			client.send(
				{Http::Method::GET,
				 "/echo/",
				 {{{"Request-Test-Header", "test"}}},
				 "some body",
				 Http::Version::_0_9});
			auto res = client.recv();
			assert(res.version == Http::Version::_0_9);
			assert(res.headers.empty());
			std::stringstream stream;
			stream << res.body;
			assert(stream.str().empty());

			// This will throw since 0.9 server has already disconnected.
			Rain::Error::consumeThrowable([&client]() { client.shutdown(); })();
		}

		// Follow-up chunked request.
		{
			MyClient client({"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;
			std::string body = "hello world";

			{
				client.send({Http::Method::POST, "/echo", {}, body});
				auto res = client.recv();
				std::stringstream stream;
				stream << res.body;
				assert(stream.str() == body);
				assert(res.headers["yOuR-MeTHOd"] == "POST");
			}

			{
				client.send({Http::Method::GET, "/echo", {}});
				auto res = client.recv();
				assert(res.headers["Your-Method"] == "GET");
			}

			{
				client.send(
					{Http::Method::POST,
					 "/echo",
					 {{{"Transfer-Encoding", "chunked"}}},
					 "5\r\nhello\r\n1\r\n \r\n5\r\nworld\r\n0\r\n\r\n"});
				auto res = client.recv();
				std::stringstream stream;
				stream << res.body;
				assert(stream.str() == body);
				assert(res.headers["your-method"] == "POST");
			}
		}

		// Idling over 5s will close connection.
		{
			MyClient client({"localhost", server.host().service});
			std::cout << client.host() << " connected to " << client.peerHost()
								<< std::endl
								<< std::endl;

			std::this_thread::sleep_for(6s);
			client.send({Http::Method::GET, "/"s});
			auto res = client.recv();
			assert(!client.good());
		}

		// Keep server open for 60s for playtest.
		std::cout << "Serving on " << server.host() << std::endl;
		std::this_thread::sleep_for(60s);
	}

	return 0;
}

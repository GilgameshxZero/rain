// Tests Networking::Http::Server.
#include <rain.hpp>

using Rain::Error::releaseAssert;
using namespace Rain::Literal;
using namespace Rain::Networking;

// Custom Request/Response output to cout when received
// via pp-chain.
class MyRequest : public Http::Request {
	public:
	using Request::Request;

	virtual void recvWith(std::istream &stream) override {
		Request::recvWith(stream);
		std::cout << "Request without body:\n"
							<< this->method << ' ' << this->target << ' '
							<< this->version << '\n'
							<< this->headers << std::endl;
	}
};
class MyResponse : public Http::Response {
	public:
	using Response::Response;

	virtual void recvWith(std::istream &stream) override {
		Response::recvWith(stream);
		std::cout << "Response without body:\n"
							<< this->version << ' ' << this->statusCode
							<< ' ' << this->reasonPhrase << '\n'
							<< this->headers << std::endl;
	}
};

class MyWorker :
	public Http::Worker<
		MyRequest,
		Http::Response,
		Ipv6FamilyInterface,
		NoLingerSocketOption> {
	public:
	// Custom currying constructor to set send/recv timeout
	// to 300ms.
	MyWorker(auto &&...args) :
		Worker(
			1_zu << 10_zu,
			1_zu << 10_zu,
			300LL,
			300LL,
			std::forward<decltype(args)>(args)...) {}

	private:
	ResponseAction reqSimple(Request &, std::smatch const &) {
		return {{StatusCode::OK}};
	}
	ResponseAction reqEcho(
		Request &req,
		std::smatch const &) {
		// Must read full body into memory, since req.body is
		// of indeterminate length.
		std::stringstream stream;
		stream << req.body;

		return {
			{StatusCode::OK,
				{{{"Your-Method", req.method},
					{"Response-Test-Header", "test"}}},
				stream.str(),
				"reason"}};
	}
	ResponseAction reqPlay(
		Request &req,
		std::smatch const &) {
		return {
			{StatusCode::OK,
				{{{"Content-Type", "text/plain; charset=UTF-8"}}},
				"Your Method: "s +
					static_cast<std::string>(req.method) +
					"\nHello world!"s,
				"bruh"}};
	}

	virtual std::vector<RequestFilter> const &
		filters() override {
		static std::vector<RequestFilter> const filters{
			{".*",
				"/simple/?",
				{Method::GET},
				&MyWorker::reqSimple},
			{".*",
				"/echo/?",
				{Method::GET, Method::POST},
				&MyWorker::reqEcho},
			{".*",
				"/play.*",
				{Method::GET, Method::POST},
				&MyWorker::reqPlay}};
		return filters;
	}

	// Override send pp-chain to add server signature.
	virtual void send(Http::Response &res) override {
		res.headers.server("MyServer");
		Worker::send(res);
	}
};

class MyServer :
	public Http::Server<
		MyWorker,
		Ipv6FamilyInterface,
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

class MyClient :
	public Http::Client<
		Http::Request,
		MyResponse,
		Ipv4FamilyInterface,
		NoLingerSocketOption> {
	public:
	// Custom currying constructor to set send/recv timeout
	// to 300ms.
	MyClient(auto &&...args) :
		Client(
			1_zu << 10_zu,
			1_zu << 10_zu,
			300LL,
			300LL,
			std::forward<decltype(args)>(args)...) {}
};

int main() {

	{
		MyServer server(":0");
		std::cout << "Serving on " << server.host()
							<< std::endl;

		// Basic request to /simple.
		{
			MyClient client(
				Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to "
								<< client.peerHost() << std::endl
								<< std::endl;
			client.send({Http::Method::GET, "/simple"s});
			auto res = client.recv();
			releaseAssert(res.version == Http::Version::_1_1);
			releaseAssert(res.statusCode == Http::StatusCode::OK);
			releaseAssert(res.headers.contentLength() == 0);
			releaseAssert(res.headers.server() == "MyServer");
			client.shutdown();
		}

		// Basic 404ing request.
		{
			MyClient client(
				Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to "
								<< client.peerHost() << std::endl
								<< std::endl;
			client.send({Http::Method::GET, "/does-not-exist"s});
			auto res = client.recv();
			releaseAssert(res.version == Http::Version::_1_1);
			releaseAssert(
				res.statusCode == Http::StatusCode::NOT_FOUND);
			releaseAssert(res.headers.contentLength() == 0);
			releaseAssert(res.headers.server() == "MyServer");
			client.shutdown();
		}

		// Server and client ignore 0.9 headers and server
		// ignores body.
		{
			MyClient client(
				Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to "
								<< client.peerHost() << std::endl
								<< std::endl;
			client.send(
				{Http::Method::GET,
					"/echo/",
					{{{"Request-Test-Header", "test"}}},
					"some body",
					Http::Version::_0_9});
			auto res = client.recv();
			releaseAssert(res.version == Http::Version::_0_9);
			releaseAssert(res.headers.empty());
			std::stringstream stream;
			stream << res.body;
			releaseAssert(stream.str().empty());

			// This will throw since 0.9 server has already
			// disconnected.
			Rain::Error::consumeThrowable(
				[&client]() { client.shutdown(); })();
		}

		// Follow-up chunked request.
		{
			MyClient client(
				Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to "
								<< client.peerHost() << std::endl
								<< std::endl;
			std::string body = "hello world";

			{
				client.send(
					{Http::Method::POST, "/echo", {}, body});
				auto res = client.recv();
				std::stringstream stream;
				stream << res.body;
				releaseAssert(stream.str() == body);
				releaseAssert(res.headers["yOuR-MeTHOd"] == "POST");
			}

			{
				client.send({Http::Method::GET, "/echo", {}});
				auto res = client.recv();
				releaseAssert(res.headers["Your-Method"] == "GET");
			}

			{
				client.send(
					{Http::Method::POST,
						"/echo",
						{{{"Transfer-Encoding", "chunked"}}},
						"5\r\nhello\r\n1\r\n "
						"\r\n5\r\nworld\r\n0\r\n\r\n"});
				auto res = client.recv();
				std::stringstream stream;
				stream << res.body;
				releaseAssert(stream.str() == body);
				releaseAssert(res.headers["your-method"] == "POST");
			}
		}

		// Idling over 300ms will close connection.
		{
			MyClient client(
				Host{"localhost", server.host().service});
			std::cout << client.host() << " connected to "
								<< client.peerHost() << std::endl
								<< std::endl;

			std::this_thread::sleep_for(310ms);
			client.send({Http::Method::GET, "/"s});
			auto res = client.recv();
			releaseAssert(!client.good());
		}
	}

	return 0;
}

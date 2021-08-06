// Tests Networking::Http::Request.
#include <rain/time/time.hpp>

#include <rain/networking/http/request.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	// List-initializing a Request with some default arguments and a body.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		std::string body("a few bytes");
		Request req{
			{},
			"/",
			{{{"Host", "google.com"}, {"Transfer-Encoding", "identity"}}},
			{body},
			Version::_1_1};
		req.headers.contentLength(body.length());

		std::stringstream stream;
		stream << req;
		// This is also acceptable.
		// req.sendWith(stream);
		std::cout << stream.str() << std::endl;

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed < 0.1s);
	}

	// Receiving a request.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		std::stringstream stream(
			"POST /not-a-real-uri HTTP/1.1\r\nHost: google.com\r\nTransfer-Encoding: "
			"identity\r\nContent-Length: 11\r\n\r\na few bytes");
		Request req;
		stream >> req;

		assert(req.method == Method::POST);
		assert(req.target == "/not-a-real-uri");
		assert(req.version == Version::_1_1);

		assert(req.headers.size() == 3);
		assert(req.headers.host().node == "google.com");
		assert(req.headers.transferEncoding().size() == 1);
		assert(
			req.headers.transferEncoding()[0] == Header::TransferEncoding::IDENTITY);
		assert(req.headers.contentLength() == 11);

		std::string buffer(1_zu << 2, '\0'), body;
		while (std::getline(req.body, buffer)) {
			body += buffer;
		}
		assert(body == "a few bytes");

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl << std::endl;
		assert(timeElapsed < 0.1s);
	}

	return 0;
}

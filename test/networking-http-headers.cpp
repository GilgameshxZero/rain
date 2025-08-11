// Test for Networking::Http::Headers.
#include <rain.hpp>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	// Case agnostic access, host wrap/unwrap.
	{
		Headers headers;

		headers.host({"google.com"});
		assert(headers["Host"] == "google.com");
		assert(headers["host"] == "google.com");
		assert(headers["hOsT"] == "google.com");
		assert(static_cast<std::string>(headers.host()) == "google.com");

		headers.host({"google.com:"});
		assert(headers["Host"] == "google.com");

		headers.host({"google.com:80"});
		assert(headers["Host"] == "google.com:80");

		headers.host({":80"});
		assert(headers.host().node == "");
		assert(headers.host().service == "80");
	}

	// Content-Length wrap/unwrap and list initialization.
	{
		Headers headers{
			{{"Content-Length", "150"},
			 {"some-random-ass-header", "bleh"},
			 {"Host", "google.com"}}};
		assert(headers.contentLength() == 150);
		headers.contentLength(0);
		assert(headers.contentLength() == 0);
		assert(headers.host().node == "google.com");
		assert(headers.host().service == "");
		assert(headers["some-random-ass-header"] == "bleh");
	}

	// Transfer-Encoding wrap/unwrap.
	{
		Headers headers{{{"Transfer-Encoding", "gzip, chunked"}}};
		assert(headers.transferEncoding().size() == 2);
		assert(headers.transferEncoding()[0] == Header::TransferEncoding::GZIP);
		assert(headers.transferEncoding()[1] == Header::TransferEncoding::CHUNKED);
	}

	return 0;
}

// Test for Networking::Http::Headers.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	// Case agnostic access, host wrap/unwrap.
	{
		Headers headers;

		headers.host({"google.com"});
		releaseAssert(headers["Host"] == "google.com");
		releaseAssert(headers["host"] == "google.com");
		releaseAssert(headers["hOsT"] == "google.com");
		releaseAssert(
			static_cast<std::string>(headers.host()) ==
			"google.com");

		headers.host({"google.com:"});
		releaseAssert(headers["Host"] == "google.com");

		headers.host({"google.com:80"});
		releaseAssert(headers["Host"] == "google.com:80");

		headers.host({":80"});
		releaseAssert(headers.host().node == "");
		releaseAssert(headers.host().service == "80");
	}

	// Content-Length wrap/unwrap and list initialization.
	{
		Headers headers{
			{{"Content-Length", "150"},
				{"some-random-ass-header", "bleh"},
				{"Host", "google.com"}}};
		releaseAssert(headers.contentLength() == 150);
		headers.contentLength(0);
		releaseAssert(headers.contentLength() == 0);
		releaseAssert(headers.host().node == "google.com");
		releaseAssert(headers.host().service == "");
		releaseAssert(
			headers["some-random-ass-header"] == "bleh");
	}

	// Transfer-Encoding wrap/unwrap.
	{
		Headers headers{
			{{"Transfer-Encoding", "gzip, chunked"}}};
		releaseAssert(headers.transferEncoding().size() == 2);
		releaseAssert(
			headers.transferEncoding()[0] ==
			Header::TransferEncoding::GZIP);
		releaseAssert(
			headers.transferEncoding()[1] ==
			Header::TransferEncoding::CHUNKED);
	}

	return 0;
}

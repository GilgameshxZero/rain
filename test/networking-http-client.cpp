// Very simple tests for Networking::Http::Client.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;
	using namespace Http;
	using namespace std;

	{
		Http::Client<> client("facebook.com:80");
		client << Http::Request();
		Response res;
		client >> res;

		cout << "HTTP/" << res.version << " " << res.statusCode
				 << " " << res.reasonPhrase << "\r\n"
				 << res.headers << std::endl;
		releaseAssert(res.version == Http::Version::_1_1);
		releaseAssert(
			res.statusCode ==
			Http::StatusCode::MOVED_PERMANENTLY);
	}

	{
		Http::Client<> client("api.ipify.org:80");
		client << Http::Request(
			Method::GET,
			"/",
			{{{"Host", "api.ipify.org"}}},
			{},
			Version::_1_1);
		Response res;
		client >> res;

		cout << res.body << std::endl;
		releaseAssert(res.version == Version::_1_1);
		releaseAssert(res.statusCode == StatusCode::OK);
	}

	// On Windows, this is accomplished via a single "bind".
	// On Linux, we additionally need SO_BINDTODEVICE because
	// it is a weak E/S model.

	// {
	// 	Http::Client<> client("api.ipify.org:80",
	// "10.8.45.51"); 	client << Http::Request( Method::GET,
	// 		"/",
	// 		{{{"Host", "api.ipify.org"}}},
	// 		{},
	// 		Version::_1_1);
	// 	Response res;
	// 	client >> res;

	// 	cout << res.body << std::endl;
	// 	releaseAssert(res.version == Version::_1_1);
	// 	releaseAssert(res.statusCode == StatusCode::OK);
	// }

	// {
	// 	IP_ADAPTER_ADDRESSES addresses;
	// 	ULONG addressesLen;
	// 	GetAdaptersAddresses(
	// 		AF_UNSPEC, 0, NULL, &addresses, &addressesLen);
	// }
	return 0;
}

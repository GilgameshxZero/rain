// Very simple tests for Networking::Http::Client.
#include <rain/networking/http/client.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking::Http;

	{
		Client client;
		client.connect({"google.com:80"});
		client << Request{};
		Response res = client.recvResponse();

		std::cout << "HTTP/" << res.version << " " << res.statusCode << " "
							<< res.reasonPhrase << "\r\n"
							<< res.headers << "\r\n"
							<< std::endl;
		assert(res.version == Version::_1_1);
		assert(res.statusCode == StatusCode::OK);
	}
	return 0;
}

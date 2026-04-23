// Very simple tests for Networking::Http::Client.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	{
		Http::Client<
			Http::Request,
			Http::Response,
			1_zu << 10,
			1_zu << 10,
			15000,
			15000,
			Ipv4FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			NoLingerSocketOption>
			client("facebook.com:80");
		client << Http::Request();
		Http::Response res;
		client >> res;

		std::cout << "HTTP/" << res.version << " "
							<< res.statusCode << " " << res.reasonPhrase
							<< "\r\n"
							<< res.headers << std::endl;
		releaseAssert(res.version == Http::Version::_1_1);
		releaseAssert(
			res.statusCode ==
			Http::StatusCode::MOVED_PERMANENTLY);
	}
	return 0;
}

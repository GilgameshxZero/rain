#include <rain.hpp>

using namespace Rain;
using namespace Networking;
using namespace Tls;
using namespace std;

void showHexStr(string &res) {
	cout << "[" << hex << setw(4) << setfill('0')
			 << res.length() << "] ";
	for (auto &i : res) {
		cout << hex << setw(2) << (int)(uint8_t)i << " ";
	}
	cout << endl;
}

int main() {
	{
		Http::Client<> httpClient({"reddit.com", 443});
		Tls::Client<Http::Client<>> tlsClient(
			std::move(httpClient));

		auto clientHello{TlsPlaintext<Handshake<ClientHello>>{
			TlsPlaintextContentType::HANDSHAKE,
			{3, 1},
			{{{3, 3},
				{},
				0,
				{CipherSuite::TLS_RSA_WITH_AES_128_CBC_SHA},
				{0},
				{}}}}};
		clientHello.sendWith(tlsClient);
		tlsClient.flush();
		string res;
		tlsClient.recv(res);
		showHexStr(res);
	}

	return 0;
}

#include <rain.hpp>

using namespace Rain;
using namespace Networking;
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
		Http::Client<> httpClient({"facebook.com", 443});
		Tls::Client<Http::Client<>> tlsClient(
			std::move(httpClient));
		tlsClient.send("\x16\x03\x01\x00\x00"s);
		string res;
		tlsClient.recv(res);
		showHexStr(res);
		tlsClient.shutdown();
	}

	return 0;
}

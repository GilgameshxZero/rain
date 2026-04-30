#include <rain.hpp>

using namespace Rain;
using namespace Networking;
using namespace Tls;
using namespace std;

void showHexStr(string const &res) {
	cout << "[" << hex << setw(4) << setfill('0')
			 << res.length() << "]\t";
	for (auto &i : res) {
		cout << hex << setw(2) << (int)(uint8_t)i << "\t";
	}
	cout << dec << setfill(' ');
}

// PP-chain by deriving Plaintext/* classes.
class MyPlaintext : public Plaintext {
	using Plaintext::Plaintext;

	public:
	// PP-chains.
	MyPlaintext(std::istream &stream) : Plaintext(stream) {
		std::stringstream ss;
		Plaintext::sendWith(ss);
		cout << "RECV ";
		showHexStr(ss.str());
		cout << endl;
	}

	virtual void sendWith(ostream &stream) const override {
		std::stringstream ss;
		Plaintext::sendWith(ss);
		cout << "SEND ";
		showHexStr(ss.str());
		cout << endl;
		Plaintext::sendWith(stream);
	}
};

int main() {
	Tls::Client<Http::Client<>> client({"google.com", 443});
	client << MyPlaintext(
		ProtocolVersion::_1_0,
		new Handshake(new ClientHello(
			ProtocolVersion::_1_2,
			{{}},
			0,
			{CipherSuite::TLS_RSA_WITH_AES_128_CBC_SHA,
				CipherSuite::TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
				CipherSuite::
					TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256},
			{0},
			{})));
	MyPlaintext response(client);
	return 0;
}

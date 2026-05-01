#include <rain.hpp>

using namespace Rain;
using namespace Networking;
using namespace Tls;
using namespace std;
using Rain::Error::releaseAssert;

bool doHandshake(
	Tls::Client<Http::Client<>> &client,
	ostream &log) {
	stringstream ss, buffer;

	// Receive everything.
	while (client) {
		string tmp;
		client.recv(tmp, 2s);
		if (tmp.empty()) {
			break;
		}
		buffer << tmp;
	}

	log << String::asHexStr(buffer.str().substr(0, 96))
			<< endl;

	{
		// Play it back.
		Plaintext plaintext(buffer);

		plaintext.sendWith(ss);
		log << String::asHexStr(ss.str()) << endl;
		ss.str("");

		if (
			plaintext.fragment->contentType() !=
			ContentType::HANDSHAKE) {
			log << "Failed to parse ServerHello::Plaintext.\n";
			return true;
		}
		auto handshake{
			dynamic_cast<Handshake *>(plaintext.fragment.get())};
		if (
			handshake->body->handshakeType() !=
			HandshakeType::SERVER_HELLO) {
			log << "Failed to parse ServerHello::Handshake.\n";
			return true;
		}
		auto serverHello{
			dynamic_cast<ServerHello *>(handshake->body.get())};
		log << "Cipher: " << hex << setfill('0') << setw(4)
				<< static_cast<uint16_t>(serverHello->cipherSuite)
				<< "." << dec << setfill(' ') << endl;
	}

	// {
	// 	Plaintext plaintext(buffer);
	// 	if (
	// 		plaintext.fragment->contentType() !=
	// 		ContentType::HANDSHAKE) {
	// 		log << "Failed to parse Certificate::Plaintext.\n";
	// 		return true;
	// 	}
	// 	auto handshake{
	// 		dynamic_cast<Handshake *>(plaintext.fragment.get())};
	// 	if (
	// 		handshake->body->handshakeType() !=
	// 		HandshakeType::CERTIFICATE) {
	// 		log << "Failed to parse Certificate::Handshake.\n";
	// 		return true;
	// 	}
	// 	auto certificate{
	// 		dynamic_cast<Certificate *>(handshake->body.get())};
	// 	log << "Certificates: "
	// 			<< certificate->certificates.size() << "." << endl;
	// }

	return false;
}

auto makeTask(
	string const &hostNode,
	vector<string> &failures) {
	static mutex failureMtx;
	return [&hostNode, &failures]() {
		stringstream log, ss;
		log << hostNode << endl;

		Tls::Client<Http::Client<>> client(Host{hostNode, 443});
		auto clientHello{Plaintext(
			ProtocolVersion::_1_0,
			new Handshake(new ClientHello(
				ProtocolVersion::_1_2,
				{{}},
				{{}},
				{CipherSuite::TLS_RSA_WITH_AES_128_CBC_SHA,
					CipherSuite::
						TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
					CipherSuite::
						TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384},
				{0},
				{{new Extension::ServerName({hostNode}),
					new Extension::SupportedGroups(
						{Extension::SupportedGroups::SECP256R1,
							Extension::SupportedGroups::SECP384R1}),
					new Extension::SignatureAlgorithms(
						{Extension::SignatureAlgorithms::
								RSA_PKCS1_SHA256,
							Extension::SignatureAlgorithms::
								ECDSA_SECP256R1_SHA256})}})))};

		clientHello.sendWith(ss);
		log << String::asHexStr(ss.str()) << endl;
		ss.str("");

		client << clientHello;

		bool fail{doHandshake(client, log)};
		if (fail) {
			lock_guard failureMtxLg(failureMtx);
			failures.push_back(hostNode);
		}
		cout << log.rdbuf() << endl;
	};
}

int main() {
	vector<string> hostNodes{
		{"google.com",
			"wikipedia.org",
			"facebook.com",
			"github.com",
			"youtube.com",
			"bandcamp.com",
			"pixiv.net",
			"myanimelist.net",
			"startpage.com",
			"reddit.com",
			"projecteuler.net",
			"amazon.com",
			"visualstudio.com",
			"twitch.tv",
			"leagueoflegends.com",
			"x.com",
			"discord.com",
			"stackoverflow.com",
			"usaco.training",
			"usaco.org",
			"codeforces.com",
			"tiktok.com",
			"hp.com",
			"quora.com",
			"bilibili.com",
			"baidu.com",
			"mihoyo.com",
			"usa.gov",
			"whitehouse.gov",
			"adobe.com",
			"alpaca.markets",
			"archive.org",
			"archlinux.org",
			"armorgames.com",
			"artofproblemsolving.com",
			"chase.com",
			"numer.ai",
			"parsec.app",
			"cs.rin.ru",
			"spoj.com",
			"zoom.us",
			"qq.com",
			"nytimes.com",
			"mitfcu.org",
			"mega.nz",
			"lichess.org",
			"jlist.com",
			"xnxx.com",
			"pornhub.com",
			"nhentai.net",
			"xvideos.com",
			"hellopoetry.com",
			"goodreads.com",
			"ebay.com",
			"deshaw.com",
			"boox.com",
			"atcoder.jp",
			"www.gov.cn",
			"empireblue.com",
			"cses.fi"}},
		failures;

	Multithreading::ThreadPool tp;
	for (auto &hostNode : hostNodes) {
		tp.queueTask(makeTask(hostNode, failures));
	}
	tp.blockForTasks();

	cout << "Passed " << hostNodes.size() - failures.size()
			 << " of " << hostNodes.size() << "." << endl
			 << "Failures: " << failures << "." << endl;
	releaseAssert(failures.empty());
	return 0;
}

// Test the asynchronous getAddressInfo from rain/networking/resolve.hpp.
#include <rain/literal.hpp>

#include <rain/networking/resolve.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// google.com should have both IPv4 and IPv6 addresses.
	{
		auto addressInfos = Resolve::getAddressInfo({"google.com"});
		std::vector<Host> hosts;
		for (Resolve::AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(
				Resolve::getNumericHost(addressInfo.address, addressInfo.addressLen));
		}

		// At least 1 IPv4.
		assert(hosts.size() >= 1);

		// Port was left unspecified.
		for (Host const &host : hosts) {
			assert(host.service == "0");
		}
	}

	// Specifying port on the Host.
	{
		auto addressInfos = Resolve::getAddressInfo({"cock.li:80"});
		std::vector<Host> hosts;
		for (Resolve::AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(
				Resolve::getNumericHost(addressInfo.address, addressInfo.addressLen));
		}

		for (Host const &host : hosts) {
			assert(host.service == "80");
		}
	}

	// PASSIVE binding address ":0". Cannot use ":" since that leaves both
	// node/service as nullptr.
	{
		auto addressInfos = Resolve::getAddressInfo(
			{":0"},
			{15s},
			{},
			Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL |
				Resolve::AddressInfoFlag::PASSIVE);
		std::vector<Host> hosts;
		for (Resolve::AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(
				Resolve::getNumericHost(addressInfo.address, addressInfo.addressLen));
		}

		// Dual stack IPv4 & IPv6.
		assert(hosts.size() >= 2);
	}

	// Addresses for connecting to localhost.
	{
		auto addressInfos = Resolve::getAddressInfo({"localhost:80"});
		std::vector<Host> hosts;
		for (Resolve::AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(
				Resolve::getNumericHost(addressInfo.address, addressInfo.addressLen));
		}

		assert(!hosts.empty());
	}

	// Zero timeout should return immediately and no hosts.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto addressInfos = Resolve::getAddressInfo({"google.com:80"}, {0s});
		std::vector<Host> hosts;
		for (Resolve::AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(
				Resolve::getNumericHost(addressInfo.address, addressInfo.addressLen));
		}

		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		assert(timeElapsed < 0.1s);
		assert(hosts.empty());
	}

	// MX record lookup.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = Resolve::getDnsRecordsMx({"cock.li"});
		for (auto const &mxRecord : mxRecords) {
			std::cout << mxRecord.first << " " << mxRecord.second << std::endl;
		}
		assert(mxRecords.size() == 2);
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;
		assert(timeElapsed < 1s);
	}

	// MX record lookup.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = Resolve::getDnsRecordsMx({"gmail.com"});
		for (auto const &mxRecord : mxRecords) {
			std::cout << mxRecord.first << " " << mxRecord.second << std::endl;
		}
		assert(mxRecords.size() == 5);
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;
		assert(timeElapsed < 1s);
	}

	// MX record timeout.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = Resolve::getDnsRecordsMx({"gmail.com"}, {0s});
		assert(mxRecords.size() == 0);
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;
		assert(timeElapsed < 0.1s);
	}

	// MX record error.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = Resolve::getDnsRecordsMx({"not-a-real-domain-name"});
		assert(mxRecords.size() == 0);
		auto timeElapsed = std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed << std::endl;

		// Failed DNS lookups can take a while on Windows.
		assert(timeElapsed < 2s);
	}

	// Sleep for resolve threads to cleanup, so that temporal memory leaks don't
	// show on valgrind.
	std::this_thread::sleep_for(1s);

	return 0;
}

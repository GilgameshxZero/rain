// Test the asynchronous getAddressInfo from
// rain/networking/resolve.hpp.
#include <rain.hpp>

using Rain::Error::releaseAssert;

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	// google.com should have both IPv4 and IPv6 addresses.
	{
		auto addressInfos = getAddressInfos({"google.com"});
		std::vector<Host> hosts;
		for (AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(getNumericHost(
				addressInfo.address, addressInfo.addressLen));
		}

		// At least 1 IPv4.
		releaseAssert(hosts.size() >= 1);

		// Port was left unspecified.
		for (Host const &host : hosts) {
			releaseAssert(host.service == "0");
		}
	}

	// Specifying port on the Host.
	{
		auto addressInfos = getAddressInfos({"cock.li:80"});
		std::vector<Host> hosts;
		for (AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(getNumericHost(
				addressInfo.address, addressInfo.addressLen));
		}

		for (Host const &host : hosts) {
			releaseAssert(host.service == "80");
		}
	}

	// PASSIVE binding address ":0". Cannot use ":" since that
	// leaves both node/service as nullptr.
	{
		auto addressInfos = getAddressInfos(
			{":0"},
			{},
			{},
			{},
			AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG |
				AddressInfo::Flag::ALL |
				AddressInfo::Flag::PASSIVE);
		std::vector<Host> hosts;
		for (AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(getNumericHost(
				addressInfo.address, addressInfo.addressLen));
		}

		// Dual stack IPv4 & IPv6.
		releaseAssert(hosts.size() >= 2);
	}

	// Addresses for connecting to localhost.
	{
		auto addressInfos = getAddressInfos({"localhost:80"});
		std::vector<Host> hosts;
		for (AddressInfo const &addressInfo : addressInfos) {
			hosts.emplace_back(getNumericHost(
				addressInfo.address, addressInfo.addressLen));
		}

		releaseAssert(!hosts.empty());
	}

	// Error domain name.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto addressInfos =
			getAddressInfos({"not-a-real-domain-name"});
		releaseAssert(addressInfos.size() == 0);
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed
							<< std::endl;

		// Failed DNS lookups can take a while.
		releaseAssert(timeElapsed < 10s);
	}

	// MX record lookup.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = getMxRecords({"cock.li"});
		for (auto const &mxRecord : mxRecords) {
			std::cout << mxRecord.first << " " << mxRecord.second
								<< std::endl;
		}
		releaseAssert(mxRecords.size() == 2);
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed
							<< std::endl;
		releaseAssert(timeElapsed < 5s);
	}

	// MX record lookup.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		auto mxRecords = getMxRecords({"gmail.com"});
		for (auto const &mxRecord : mxRecords) {
			std::cout << mxRecord.first << " " << mxRecord.second
								<< std::endl;
		}
		releaseAssert(mxRecords.size() == 5);
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed
							<< std::endl;
		releaseAssert(timeElapsed < 2s);
	}

	// MX record error.
	{
		auto timeBegin = std::chrono::steady_clock::now();
		try {
			auto mxRecords =
				getMxRecords({"not-a-real-domain-name"});
			releaseAssert(mxRecords.size() == 0);
		} catch (std::exception const &exception) {
			std::cout << exception.what();
		}
		auto timeElapsed =
			std::chrono::steady_clock::now() - timeBegin;
		std::cout << "Time elapsed: " << timeElapsed
							<< std::endl;

		// Failed DNS lookups can take a while.
		releaseAssert(timeElapsed < 10s);
	}

	// Sleep for resolve threads to cleanup, so that temporal
	// memory leaks don't show on valgrind.
	std::this_thread::sleep_for(100ms);

	return 0;
}

// DNS/address resolution.
#pragma once

#include "../literal.hpp"
#include "exception.hpp"
#include "host.hpp"
#include "specification.hpp"

// This file uses kernel networking calls and none of its includes do.
#include "wsa.hpp"

#ifdef RAIN_PLATFORM_WINDOWS

// Windows DNS utilities.
#pragma comment(lib, "Dnsapi.lib")

#include <WinDNS.h>

#else

// POSIX libresolv.
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/types.h>

#endif

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

namespace Rain::Networking {
	// C++ equivalent of addrinfo for both IPv4 and IPv6.
	struct AddressInfo {
		enum class Flag {
			PASSIVE = AI_PASSIVE,
			CANONNAME = AI_CANONNAME,
			NUMERICHOST = AI_NUMERICHOST,
			NUMERICSRV = AI_NUMERICSERV,

			ALL = AI_ALL,
			ADDRCONFIG = AI_ADDRCONFIG,
			V4MAPPED = AI_V4MAPPED
		};
		Flag flags;
		Family family;
		Type type;
		Protocol protocol;

		std::string canonName;

		// sockaddr_storage is large enough to store both IPv4 and IPv6 socket
		// addresses.
		sockaddr_storage address;
		socklen_t addressLen;
	};

	// Custom operator for flag-like enum class.
	inline AddressInfo::Flag operator|(
		AddressInfo::Flag const &left,
		AddressInfo::Flag const &right) {
		return static_cast<AddressInfo::Flag>(
			static_cast<int>(left) | static_cast<int>(right));
	}

	// Utility function to retrieve hostname (node and service) from an address in
	// sockaddr_storage returned by getAddressInfo. Internally uses getnameinfo,
	// but with the numeric flags this should not trigger a reverse DNS lookup and
	// thus should not block.
	inline Host getNumericHost(
		sockaddr_storage const &address,
		socklen_t addressLen) {
		char node[NI_MAXHOST], service[NI_MAXSERV];

		// reinterpret_cast here does not break strict aliasing, since we do not
		// dereference. That job is up to getnameinfo's implementation.
		if (
			getnameinfo(
				reinterpret_cast<sockaddr const *>(&address),
				addressLen,
				node,
				static_cast<socklen_t>(sizeof(node)),
				service,
				static_cast<socklen_t>(sizeof(service)),
				NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			throw Networking::Exception(Networking::getSystemError());
		}

		// Guaranteed RVO.
		return {node, service};
	}
	inline Host getNumericHost(AddressInfo const &addressInfo) {
		return getNumericHost(addressInfo.address, addressInfo.addressLen);
	}

	// Calls (blocking) getaddrinfo.
	//
	// Usually does not throw, even on invalid lookups.
	inline std::vector<AddressInfo> getAddressInfos(
		Host const &host,
		Family family = Family::UNSPEC,
		Type type = Type::ANY,
		Protocol protocol = Protocol::ANY,
		AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
			AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL) {
		std::vector<AddressInfo> addressInfos;

		addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_flags = static_cast<int>(flags);
		hints.ai_family = static_cast<int>(family);
		hints.ai_socktype = static_cast<int>(type);
		hints.ai_protocol = static_cast<int>(protocol);

		addrinfo *addresses = nullptr;
		int result = getaddrinfo(
			host.node.empty() ? nullptr : host.node.c_str(),
			host.service.empty() ? nullptr : host.service.c_str(),
			&hints,
			&addresses);

		if (result == -1) {
			// getaddrinfo error.
			if (addresses != nullptr) {
				freeaddrinfo(addresses);
			}
			throw Networking::Exception(static_cast<Networking::Error>(result));
		} else {
			addrinfo *curAddr;

			// getaddrinfo completed successfully. Transcribe all addresses, then
			// free.
			curAddr = addresses;
			while (curAddr != nullptr) {
				addressInfos.push_back(
					{static_cast<AddressInfo::Flag>(curAddr->ai_flags),
					 static_cast<Family>(curAddr->ai_family),
					 static_cast<Type>(curAddr->ai_socktype),
					 static_cast<Protocol>(curAddr->ai_protocol),
					 {curAddr->ai_canonname == nullptr ? "" : curAddr->ai_canonname},
					 // memcpy sockaddr_storage later.
					 {},
					 static_cast<socklen_t>(curAddr->ai_addrlen)});
				std::memcpy(
					&addressInfos.back().address, curAddr->ai_addr, curAddr->ai_addrlen);
				curAddr = curAddr->ai_next;
			}

			freeaddrinfo(addresses);
		}

		return addressInfos;
	}

	// Gets MX records for a host, and sorts them in order of priority.
	inline std::vector<std::pair<std::size_t, std::string>> getMxRecords(
		Host const &host) {
		std::vector<std::pair<std::size_t, std::string>> mxRecords;

#ifdef RAIN_PLATFORM_WINDOWS
		// Rain builds with UNICODE, but we will call the ANSI functions here
		// directly, since DNS records do not resolve to wide strings.

		// DnsQuery is synchronous, while DnsQueryEx is asynchronous. However,
		// since A/AAAA lookup potentially dangles a thread, we will risk that
		// here as well. In addition, the non-Windows code is necessarily
		// synchronous too, without introducing much further resolver
		// complexity.
		DNS_RECORDA *dnsRecord;
		DNS_STATUS dnsQueryStatus = DnsQuery_A(
			host.node.c_str(),
			DNS_TYPE_MX,
			DNS_QUERY_STANDARD,
			NULL,
			// Cast directly to the generic version since we are using ANSI
			// here.
			reinterpret_cast<DNS_RECORD **>(&dnsRecord),
			NULL);
		if (dnsQueryStatus == 0) {
			DNS_RECORDA *curDnsRecord = dnsRecord;
			while (curDnsRecord != NULL && curDnsRecord->wType == DNS_TYPE_MX) {
				mxRecords.emplace_back(
					curDnsRecord->Data.MX.wPreference,
					curDnsRecord->Data.MX.pNameExchange);
				curDnsRecord = curDnsRecord->pNext;
			}
			DnsRecordListFree(dnsRecord, DnsFreeRecordList);
		} else {
			throw Networking::Exception(
				static_cast<Networking::Error>(dnsQueryStatus));
		}
#else
		u_char dnsRes[NS_PACKETSZ];

		// Make the DNS MX query. Remember to use FQDN with period at end.
		int queryLen = res_query(
			(host.node + ".").c_str(), ns_c_in, ns_t_mx, dnsRes, sizeof(dnsRes));
		if (queryLen < 0) {
			throw Exception(Error::RES_QUERY_FAILED);
		}

		// Try to begin parsing the nameserver response?
		ns_msg hMsgs;
		if (ns_initparse(dnsRes, queryLen, &hMsgs) < 0) {
			throw Exception(Error::NS_INITPARSE_FAILED);
		}

		// Get the number of answer.
		int cMsg = ns_msg_count(hMsgs, ns_s_an);
		if (cMsg < 0) {
			throw Exception(Error::NS_MSG_COUNT_FAILED);
		}

		ns_rr record;
		char buffer[NS_PACKETSZ], mxNameBuffer[NS_PACKETSZ];
		for (int msgIdx = 0; msgIdx < cMsg; msgIdx++) {
			// If parsing a single message fails, move on.
			if (ns_parserr(&hMsgs, ns_s_an, msgIdx, &record)) {
				continue;
			}
			if (
				ns_sprintrr(&hMsgs, &record, NULL, NULL, buffer, sizeof(buffer)) < 0) {
				continue;
			}

			if (ns_rr_class(record) == ns_c_in && ns_rr_type(record) == ns_t_mx) {
				if (
					dn_expand(
						ns_msg_base(hMsgs),
						ns_msg_base(hMsgs) + ns_msg_size(hMsgs),
						ns_rr_rdata(record) + NS_INT16SZ,
						mxNameBuffer,
						sizeof(mxNameBuffer)) < 0) {
					continue;
				}

				mxRecords.emplace_back(ns_get16(ns_rr_rdata(record)), mxNameBuffer);
			}
		}
#endif

		// Sort by priority and return.
		std::sort(mxRecords.begin(), mxRecords.end());
		return mxRecords;
	}
}

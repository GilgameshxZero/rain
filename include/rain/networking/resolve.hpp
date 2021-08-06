// DNS/address resolution.
#pragma once

#include "../error/consume-throwable.hpp"
#include "../literal.hpp"
#include "../time.hpp"
#include "../time/timeout.hpp"
#include "exception.hpp"
#include "host.hpp"
#include "specification.hpp"
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

namespace Rain::Networking::Resolve {
	// Internal error class thrown by resolve threads. Other errors should be
	// thrown as Networking::Exceptions.
	enum class Error {
		RES_QUERY_FAILED = 1,
		NS_INITPARSE_FAILED,
		NS_MSG_COUNT_FAILED
	};
	class ErrorCategory : public std::error_category {
		public:
		char const *name() const noexcept { return "Rain::Networking::Resolve"; }
		std::string message(int error) const noexcept {
			switch (static_cast<Error>(error)) {
				case Error::RES_QUERY_FAILED:
					return "res_query failed.";
				case Error::NS_INITPARSE_FAILED:
					return "ns_initparse failed.";
				case Error::NS_MSG_COUNT_FAILED:
					return "ns_msg_count failed.";
				default:
					return "Generic.";
			}
		}
	};
	typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

	// Flags passed to getaddrinfo and the like relating to how the address info
	// is returned. What is not shown but implicit are bitwise-OR combinations of
	// the flags.
	enum class AddressInfoFlag {
		PASSIVE = AI_PASSIVE,

		ALL = AI_ALL,
		ADDRCONFIG = AI_ADDRCONFIG,
		V4MAPPED = AI_V4MAPPED
	};

	// Custom operator for this flag-like enum class.
	inline AddressInfoFlag operator|(
		AddressInfoFlag const &left,
		AddressInfoFlag const &right) {
		return static_cast<AddressInfoFlag>(
			static_cast<int>(left) | static_cast<int>(right));
	}

	// C++ equivalent of addrinfo for both IPv4 and IPv6.
	struct AddressInfo {
		AddressInfoFlag flags;

		Specification::AddressFamily addressFamily;
		Specification::SocketType socketType;
		Specification::SocketProtocol socketProtocol;

		std::string canonicalName;

		// sockaddr_storage is large enough to store both IPv4 and IPv6 socket
		// addresses.
		sockaddr_storage address;

		std::size_t addressLen;
	};

	// Utility function to retrieve hostname (node and service) from an address in
	// sockaddr_storage returned by getAddressInfo. Internally uses getnameinfo,
	// but with the numeric flags this should not trigger a reverse DNS lookup and
	// thus should not block.
	inline Host getNumericHost(sockaddr_storage const &address, std::size_t addressLen) {
		Wsa::prepare();

		char node[NI_MAXHOST], service[NI_MAXSERV];

		// reinterpret_cast here does not break strict aliasing, since we do not
		// dereference. That job is up to getnameinfo's implementation.
		if (
			getnameinfo(
				reinterpret_cast<sockaddr const *>(&address),
				static_cast<socklen_t>(addressLen),
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

	// Implements non-blocking getaddrinfo (thread-safe by default) with timeout
	// through std::thread. std::thread is not aborted but rather this function
	// returns on timeout, and the thread closes by itself.
	//
	// On timeout, returns an empty vector.
	//
	// TODO: Implement DNS resolver for cleaner timeouts (no dangling threads).
	template <typename Clock = std::chrono::steady_clock>
	inline std::vector<AddressInfo> getAddressInfo(
		Host const &host,
		Time::Timeout<Clock> const &timeout = 15s,
		Specification::Specification const &specification = {},
		AddressInfoFlag flags = AddressInfoFlag::V4MAPPED |
			AddressInfoFlag::ADDRCONFIG | AddressInfoFlag::ALL) {
		Wsa::prepare();

		// DEFAULT resolves to UNSPEC and ANY.
		Specification::Specification resolvedSpecification{
			specification.getAddressFamily() == Specification::AddressFamily::DEFAULT
				? Specification::AddressFamily::UNSPEC
				: specification.getAddressFamily(),
			specification.getSocketType() == Specification::SocketType::DEFAULT
				? Specification::SocketType::ANY
				: specification.getSocketType(),
			specification.getSocketProtocol() ==
					Specification::SocketProtocol::DEFAULT
				? Specification::SocketProtocol::ANY
				: specification.getSocketProtocol()};

		std::vector<AddressInfo> addressInfos;

		addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_flags = static_cast<int>(flags);
		hints.ai_family =
			static_cast<int>(resolvedSpecification.getAddressFamily());
		hints.ai_socktype = static_cast<int>(resolvedSpecification.getSocketType());
		hints.ai_protocol =
			static_cast<int>(resolvedSpecification.getSocketProtocol());

		// All shared resources with gai thread allocated on the stack.

		// -1 indicates gai function call has not yet completed.
		std::shared_ptr<std::atomic_int> status(new std::atomic_int(-1));

		// Holder of the mutex is using dynamically allocated memory.
		std::shared_ptr<std::mutex> memoryMtx(new std::mutex);

		// Signalled by thread when it is done.
		std::shared_ptr<std::condition_variable> gaiEv(new std::condition_variable);

		// Whether or not this wrapper function (as opposed to gai thread) should
		// free result from gai.
		std::shared_ptr<std::atomic_bool> wrapperShouldFree(
			new std::atomic_bool(true));

		std::unique_lock<std::mutex> memoryLck(*memoryMtx);

		// gai result.
		std::shared_ptr<addrinfo *> result(new addrinfo *(nullptr));

		// Copy-capture shared pointers and values which might go out of scope.
		std::thread(
			Rain::Error::consumeThrowable(
				[host, hints, status, memoryMtx, gaiEv, wrapperShouldFree, result]() {
					int statusLocal = getaddrinfo(
						host.node.empty() ? nullptr : host.node.c_str(),
						host.service.empty() ? nullptr : host.service.c_str(),
						&hints,
						&*result);

					// If lock succeeds, either wrapper function is still waiting or it
					// has exited.
					std::lock_guard<std::mutex> memoryLckGuard(*memoryMtx);
					status->store(statusLocal);
					gaiEv->notify_all();

					// If wrapper function has exited, then we are responsible for freeing
					// memory.
					if (!wrapperShouldFree->load()) {
						if (*result != nullptr) {
							freeaddrinfo(*result);
						}
					}

					// memoryMtx unlocked.
				},
				RAIN_ERROR_LOCATION))
			.detach();

		// Wait up until the timeout while protecting against spurious wakeups and
		// locking against memoryMtx.
		if (timeout.isInfinite()) {
			gaiEv->wait(memoryLck, [status]() { return status->load() != -1; });
		} else {
			gaiEv->wait_until(memoryLck, timeout.getTimeoutTime(), [status]() {
				return status->load() != -1;
			});
		}
		// memoryMtx locked. Either gai is still running, or the thread has exited.

		if (status->load() == -1) {
			// Timeout. Tell the thread it is responsible for freeing result now.
			wrapperShouldFree->store(false);
		} else {
			// Thread has finished. Process results and free memory.
			addrinfo *curAddr;

			if (*status != 0) {
				// getaddrinfo error.
				if (*result != nullptr) {
					freeaddrinfo(*result);
				}
				throw Networking::Exception(
					static_cast<Networking::Error>(status->load()));
			} else {
				// getaddrinfo completed successfully. Transcribe all addresses, then
				// free.
				curAddr = *result;
				while (curAddr != nullptr) {
					addressInfos.push_back(AddressInfo(
						{static_cast<AddressInfoFlag>(curAddr->ai_flags),
						 static_cast<Specification::AddressFamily>(curAddr->ai_family),
						 static_cast<Specification::SocketType>(curAddr->ai_socktype),
						 static_cast<Specification::SocketProtocol>(curAddr->ai_protocol),
						 curAddr->ai_canonname == nullptr ? "" : curAddr->ai_canonname,
						 // memcpy later.
						 {},
						 curAddr->ai_addrlen}));
					std::memcpy(
						&addressInfos.back().address,
						curAddr->ai_addr,
						curAddr->ai_addrlen);
					curAddr = curAddr->ai_next;
				}

				freeaddrinfo(*result);
			}
		}

		return addressInfos;
	}

	// Gets MX records for a host, and sorts them in order of priority.
	template <typename Clock = std::chrono::steady_clock>
	inline std::vector<std::pair<std::size_t, std::string>> getDnsRecordsMx(
		Host const &host,
		Time::Timeout<Clock> const &timeout = 15s) {
		Wsa::prepare();

		// We need to start a timeouttable thread.
		std::shared_ptr<std::condition_variable> ev(new std::condition_variable);
		std::shared_ptr<std::mutex> mtx(new std::mutex);
		std::unique_lock<std::mutex> lck(*mtx);
		std::shared_ptr<std::vector<std::pair<std::size_t, std::string>>> mxRecords(
			new std::vector<std::pair<std::size_t, std::string>>());
		std::shared_ptr<std::atomic_bool> done(new std::atomic_bool(false));

		// Begin querying/blocking. Must not throw.
		std::thread([host, ev, mtx, mxRecords, done]() {
			// Operate on local records, swap at end to avoid hogging mutex.
			std::vector<std::pair<std::size_t, std::string>> localMxRecords;

			Rain::Error::consumeThrowable(
				[host, ev, mtx, mxRecords, done, &localMxRecords]() {
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
							localMxRecords.emplace_back(
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
						(host.node + ".").c_str(),
						ns_c_in,
						ns_t_mx,
						dnsRes,
						sizeof(dnsRes));
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
							ns_sprintrr(&hMsgs, &record, NULL, NULL, buffer, sizeof(buffer)) <
							0) {
							continue;
						}

						if (
							ns_rr_class(record) == ns_c_in && ns_rr_type(record) == ns_t_mx) {
							if (
								dn_expand(
									ns_msg_base(hMsgs),
									ns_msg_base(hMsgs) + ns_msg_size(hMsgs),
									ns_rr_rdata(record) + NS_INT16SZ,
									mxNameBuffer,
									sizeof(mxNameBuffer)) < 0) {
								continue;
							}

							localMxRecords.emplace_back(
								ns_get16(ns_rr_rdata(record)), mxNameBuffer);
						}
					}
#endif
				},
				RAIN_ERROR_LOCATION)();
			// Throws avoid waiting the entire timeout by notifying ev.

			// Lock memory and swap in.
			std::lock_guard lckGuard(*mtx);
			mxRecords->swap(localMxRecords);
			done->store(true);

			// In any case, we are done, so notify now.
			ev->notify_all();
		}).detach();

		// Wait up to timeout.
		if (timeout.isInfinite()) {
			ev->wait(lck, [done]() { return done->load() == true; });
		} else {
			ev->wait_until(lck, timeout.getTimeoutTime(), [done]() {
				return done->load() == true;
			});
		}

		// Sort by priority and return.
		std::sort(mxRecords->begin(), mxRecords->end());
		return *mxRecords;
	}
}

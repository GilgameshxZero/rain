// SMTP Socket subclassing R/R Socket.
#pragma once

#include "../request-response/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Smtp {
	// SMTP Socket subclassing R/R Socket. Uses SMTP R/R and default R/R Socket
	// clock.
	class Socket : public RequestResponse::Socket<Request, Response> {
		public:
		typedef RequestResponse::Socket<Request, Response> SuperSocket;

		// Bring dependent aliases into scope.
		using typename SuperSocket::Request;
		using typename SuperSocket::Response;
		using typename SuperSocket::Clock;
		using typename SuperSocket::Duration;
		using typename SuperSocket::Message;

		// Same constructor.
		Socket(
			Specification::Specification,
			bool interruptable = true,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s)
				: SuperSocket(
						Specification::Specification(),
						interruptable,
						pf,
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration) {}
		Socket(Socket const &) = delete;
		Socket &operator=(Socket const &) = delete;
		Socket(Socket &&other) : SuperSocket(std::move(other)) {}

		Socket(
			Networking::Socket &&socket,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s)
				: SuperSocket(
						std::move(socket),
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration) {}

		// This connect also resolves MX records on host and connects to those.
		template <typename Clock = std::chrono::steady_clock>
		bool connectMx(
			Host const &host,
			bool parallel = true,
			Time::Timeout<Clock> const &timeout = 60s,
			Specification::Specification const &gaiSpecification = {},
			Resolve::AddressInfoFlag gaiFlags = Resolve::AddressInfoFlag::V4MAPPED |
				Resolve::AddressInfoFlag::ADDRCONFIG | Resolve::AddressInfoFlag::ALL) {
			// Resolve MX records.
			auto mxRecords = Resolve::getDnsRecordsMx(host, timeout);

			// Resolve all records into addresses.
			std::vector<Resolve::AddressInfo> addressInfos;
			for (auto const &mxRecord : mxRecords) {
				auto newAddresses = Resolve::getAddressInfo(
					{mxRecord.second, host.service},
					timeout,
					Specification::Specification(
						this->getSpecification(), gaiSpecification),
					gaiFlags);
				addressInfos.insert(
					addressInfos.end(), newAddresses.begin(), newAddresses.end());
			}

			// Connect to all the addresses.
			return this->connect(addressInfos, parallel, timeout);
		}
	};
}

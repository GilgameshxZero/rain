// HTTP Socket subclassing R/R Socket.
#pragma once

#include "../request-response/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Http {
	// HTTP Socket subclassing R/R Socket. Uses HTTP R/R and default R/R Socket
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
	};
}

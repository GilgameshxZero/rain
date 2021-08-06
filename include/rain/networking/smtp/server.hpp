// SMTP Server specialization.
#pragma once

#include "../request-response/server.hpp"
#include "socket.hpp"
#include "worker.hpp"

namespace Rain::Networking::Smtp {
	// SMTP Server specialization.
	template <typename ProtocolSocket, typename ProtocolWorker>
	class ServerInterface : public RequestResponse::
														ServerInterface<ProtocolSocket, ProtocolWorker> {
		public:
		typedef ProtocolSocket Socket;
		typedef ProtocolWorker Worker;

		// Alias Socket templates.
		using typename Socket::Request;
		using typename Socket::Response;
		using typename Socket::Clock;
		using typename Socket::Duration;
		using typename Socket::Message;

		private:
		// SuperInterface aliases the superclass.
		typedef RequestResponse::ServerInterface<Socket, Worker> SuperInterface;

		public:
		// Interface aliases this class.
		typedef ServerInterface<Socket, Worker> Interface;

		public:
		// Use the same constructor.
		template <typename... SocketArgs>
		ServerInterface(
			std::size_t maxThreads = 1024,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s,
			SocketArgs &&...args)
				: SuperInterface(
						maxThreads,
						// Relay worker construction arguments.
						pf,
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration,
						std::forward<SocketArgs>(args)...) {}
		ServerInterface(ServerInterface const &) = delete;
		ServerInterface &operator=(ServerInterface const &) = delete;

		// Worker constructor remains the same, so workerFactory does as well.
	};

	typedef ServerInterface<Socket, Worker> Server;
}

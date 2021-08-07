// Server specialization for R/R protocol Sockets.
#pragma once

#include "../tcp/server.hpp"

namespace Rain::Networking::RequestResponse {
	// Server specialization for R/R protocol Sockets.
	//
	// No support for pre/post-processing.
	template <typename ProtocolSocket, typename ProtocolWorker>
	class ServerInterface
			: public Tcp::ServerInterface<ProtocolSocket, ProtocolWorker> {
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
		typedef Tcp::ServerInterface<Socket, Worker> SuperInterface;

		public:
		// Interface aliases this class.
		typedef ServerInterface<Socket, Worker> Interface;

		protected:
		// Hold a copy of Worker constructor arguments for future construction.
		Duration const maxRecvIdleDuration, sendOnceTimeoutDuration;

		public:
		// R/R Socket parameters in Server constructor are relayed to each
		// constructed Worker.
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
						// These don't matter but must be passed for server Socket
						// construction.
						60s,
						60s,
						std::forward<SocketArgs>(args)...),
					maxRecvIdleDuration(maxRecvIdleDuration),
					sendOnceTimeoutDuration(sendOnceTimeoutDuration) {}
		ServerInterface(ServerInterface const &) = delete;
		ServerInterface &operator=(ServerInterface const &) = delete;

		private:
		// Override Worker construction with all saved Worker parameters.
		virtual std::unique_ptr<Worker> workerFactory(
			std::shared_ptr<std::pair<Networking::Socket, Resolve::AddressInfo>>
				acceptRes) override {
			return std::make_unique<Worker>(
				acceptRes->second,
				std::move(acceptRes->first),
				SuperInterface::recvBufferLen,
				SuperInterface::sendBufferLen,
				this->maxRecvIdleDuration,
				this->sendOnceTimeoutDuration);
		}
	};
}

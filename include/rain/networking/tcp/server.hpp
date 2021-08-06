// Server specialization for TCP protocol Sockets.
#pragma once

#include "../server.hpp"
#include "socket.hpp"
#include "worker.hpp"

namespace Rain::Networking::Tcp {
	// Server specialization for TCP protocol Sockets.
	template <typename ProtocolSocket, typename ProtocolWorker>
	class ServerInterface
			: public Networking::ServerInterface<ProtocolSocket, ProtocolWorker> {
		public:
		typedef ProtocolSocket Socket;
		typedef ProtocolWorker Worker;

		private:
		// SuperInterface aliases the superclass.
		typedef Networking::ServerInterface<ProtocolSocket, ProtocolWorker>
			SuperInterface;

		public:
		// Interface aliases this class.
		typedef ServerInterface<Socket, Worker> Interface;

		protected:
		// Hold a copy of Worker constructor arguments for future construction.
		std::size_t const recvBufferLen, sendBufferLen;

		public:
		// R/R Socket parameters in Server constructor are relayed to each
		// constructed Worker.
		template <typename... SocketArgs>
		ServerInterface(
			std::size_t maxThreads = 1024,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			SocketArgs &&...args)
				: SuperInterface(
						Specification::Specification(),
						maxThreads,
						pf,
						// The server does not need any sort of buffer.
						0,
						0,
						std::forward<SocketArgs>(args)...),
					recvBufferLen(recvBufferLen),
					sendBufferLen(sendBufferLen) {}
		ServerInterface(ServerInterface const &) = delete;
		ServerInterface &operator=(ServerInterface const &) = delete;

		private:
		// Calls the custom Worker/Socket constructor with saved constructor
		// arguments.
		virtual std::unique_ptr<Worker> workerFactory(
			std::shared_ptr<std::pair<Networking::Socket, Resolve::AddressInfo>> acceptRes)
			override {
			return std::make_unique<Worker>(
				acceptRes->second,
				std::move(acceptRes->first),
				this->recvBufferLen,
				this->sendBufferLen);
		}
	};

	typedef ServerInterface<Socket, Worker> Server;
}

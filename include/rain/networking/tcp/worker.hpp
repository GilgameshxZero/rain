// Worker specialization for TCP protocol Sockets.
#pragma once

#include "../worker.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	class WorkerSocketSpecInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public Networking::WorkerSocketSpecInterface {};

	// Worker specialization for TCP protocol Sockets.
	template <typename Socket>
	class WorkerSocketSpec : public Socket,
													 virtual public WorkerSocketSpecInterface {
		using Socket::Socket;
	};

	// Shorthand for TCP Worker.
	template <
		std::size_t sendBufferLen,
		std::size_t recvBufferLen,
		long long sendTimeoutMs,
		long long recvTimeoutMs,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Worker : public WorkerSocketSpec<ConnectedSocketSpec<
									 sendBufferLen,
									 recvBufferLen,
									 sendTimeoutMs,
									 recvTimeoutMs,
									 NamedSocketSpec<SocketSpec<Networking::Worker<
										 SocketFamilyInterface,
										 SocketTypeInterface,
										 SocketProtocolInterface,
										 SocketOptions...>>>>> {
		using WorkerSocketSpec<ConnectedSocketSpec<
			sendBufferLen,
			recvBufferLen,
			sendTimeoutMs,
			recvTimeoutMs,
			NamedSocketSpec<SocketSpec<Networking::Worker<
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>>::WorkerSocketSpec;
	};
}

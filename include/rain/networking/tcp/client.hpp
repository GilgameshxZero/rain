// Client specialization for TCP Sockets.
#pragma once

#include "../client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	class ClientSocketSpecInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public Networking::ClientSocketSpecInterface {};

	template <typename Socket>
	class ClientSocketSpec : public Socket,
													 virtual public ClientSocketSpecInterface {
		using Socket::Socket;
	};

	// Shorthand for TCP Client.
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
	class Client : public ClientSocketSpec<ConnectedSocketSpec<
									 sendBufferLen,
									 recvBufferLen,
									 sendTimeoutMs,
									 recvTimeoutMs,
									 NamedSocketSpec<SocketSpec<Networking::Client<
										 SocketFamilyInterface,
										 SocketTypeInterface,
										 SocketProtocolInterface,
										 SocketOptions...>>>>> {
		using ClientSocketSpec<ConnectedSocketSpec<
			sendBufferLen,
			recvBufferLen,
			sendTimeoutMs,
			recvTimeoutMs,
			NamedSocketSpec<SocketSpec<Networking::Client<
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>>::ClientSocketSpec;
	};
}
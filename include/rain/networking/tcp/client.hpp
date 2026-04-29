// Client specialization for TCP Sockets.
#pragma once

#include "../client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public Networking::
			ClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			Tcp::ConnectedSocketSpecInterface;
	};

	class ClientSocketSpecInterface :
		virtual public ClientSocketSpecInterfaceInterface,
		virtual public Networking::ClientSocketSpecInterface {
		public:
		using ClientSocketSpecInterfaceInterface =
			Tcp::ClientSocketSpecInterfaceInterface;
	};

	template<typename Socket>
	class ClientSocketSpec :
		public Socket,
		virtual public ClientSocketSpecInterface {
		using Socket::Socket;

		public:
		using ClientSocketSpecInterface =
			ClientSocketSpecInterface;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	//
	// Type/Protocol do not get template parameters as they
	// are fixed for TCP layer sockets.
	//
	// SocketFamilyInterface gets a default parameter.
	// However, to specify any of the later SocketOptions, the
	// FamilyInterface must be specified.
	template<
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Client :
		public ClientSocketSpec<ConnectedSocketSpec<
			NamedSocketSpec<SocketSpec<Networking::Client<
				SocketFamilyInterface,
				StreamTypeInterface,
				TcpProtocolInterface,
				SocketOptions...>>>>> {
		public:
		using ClientSocketSpec =
			ClientSocketSpec<ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<Networking::Client<
					SocketFamilyInterface,
					StreamTypeInterface,
					TcpProtocolInterface,
					SocketOptions...>>>>>;
		using ClientSocketSpec::ClientSocketSpec;
	};
}

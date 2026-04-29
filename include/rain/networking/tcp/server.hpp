// Server specialization for TCP protocol Sockets.
#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	class ServerSocketSpecInterfaceInterface :
		virtual public NamedSocketSpecInterface,
		virtual public Networking::
			ServerSocketSpecInterfaceInterface {};

	template<typename WorkerSocketSpec>
	class ServerSocketSpecInterface :
		virtual public ServerSocketSpecInterfaceInterface,
		virtual public Networking::ServerSocketSpecInterface<
			WorkerSocketSpec> {};

	// Server specialization for TCP protocol Sockets.
	template<typename WorkerSocketSpec, typename Socket>
	class ServerSocketSpec :
		public Socket,
		virtual public ServerSocketSpecInterface<
			WorkerSocketSpec> {
		using Socket::Socket;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	//
	// Type/Protocol do not get template parameters as they
	// are fixed for TCP layer sockets.
	template<
		typename WorkerSocketSpec,
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Server :
		public ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Networking::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				StreamTypeInterface,
				TcpProtocolInterface,
				SocketOptions...>>>> {
		public:
		using ServerSocketSpec = ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Networking::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				StreamTypeInterface,
				TcpProtocolInterface,
				SocketOptions...>>>>;
		using ServerSocketSpec::ServerSocketSpec;
	};
}

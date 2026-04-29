// Server specialization for R/R protocol Sockets.
#pragma once

#include "../tcp/server.hpp"
#include "socket.hpp"

namespace Rain::Networking::ReqRes {
	class ServerSocketSpecInterfaceInterface :
		virtual public NamedSocketSpecInterface,
		virtual public Tcp::ServerSocketSpecInterfaceInterface {
	};

	// Server specialization for R/R protocol Sockets.
	//
	// No support for pre/post-processing.
	template<typename WorkerSocketSpec>
	class ServerSocketSpecInterface :
		virtual public ServerSocketSpecInterfaceInterface,
		virtual public Tcp::ServerSocketSpecInterface<
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
	template<
		typename WorkerSocketSpec,
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Server :
		public ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Tcp::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketOptions...>>>> {
		public:
		using ServerSocketSpec = ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Tcp::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketOptions...>>>>;
		using ServerSocketSpec::ServerSocketSpec;
	};
}

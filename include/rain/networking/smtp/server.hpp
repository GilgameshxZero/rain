// SMTP Server specialization.
#pragma once

#include "../req_res/server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// SMTP Server specialization.
	class ServerSocketSpecInterfaceInterface :
		virtual public NamedSocketSpecInterface,
		virtual public ReqRes::
			ServerSocketSpecInterfaceInterface {};

	template<typename WorkerSocketSpec>
	class ServerSocketSpecInterface :
		virtual public ServerSocketSpecInterfaceInterface,
		virtual public ReqRes::ServerSocketSpecInterface<
			WorkerSocketSpec> {};

	// Server specialization for SMTP protocol Sockets.
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
			NamedSocketSpec<SocketSpec<ReqRes::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketOptions...>>>> {
		public:
		using ServerSocketSpec = ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<ReqRes::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketOptions...>>>>;
		using ServerSocketSpec::ServerSocketSpec;
	};
}

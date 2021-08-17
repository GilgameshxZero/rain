// HTTP Server specialization.
#pragma once

#include "../req-res/server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	// HTTP Server specialization.
	class ServerSocketSpecInterfaceInterface
			: virtual public NamedSocketSpecInterface,
				virtual public ReqRes::ServerSocketSpecInterfaceInterface {};

	template <typename WorkerSocketSpec>
	class ServerSocketSpecInterface
			: virtual public ServerSocketSpecInterfaceInterface,
				virtual public ReqRes::ServerSocketSpecInterface<WorkerSocketSpec> {};

	// Server specialization for HTTP protocol Sockets.
	template <typename WorkerSocketSpec, typename Socket>
	class ServerSocketSpec
			: public Socket,
				virtual public ServerSocketSpecInterface<WorkerSocketSpec>,
				virtual public ServerSocketSpecInterfaceInterface {
		using Socket::Socket;
	};

	// Shorthand for HTTP Server.
	template <
		typename WorkerSocketSpec,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Server : public ServerSocketSpec<
									 WorkerSocketSpec,
									 NamedSocketSpec<SocketSpec<ReqRes::Server<
										 WorkerSocketSpec,
										 SocketFamilyInterface,
										 SocketTypeInterface,
										 SocketProtocolInterface,
										 SocketOptions...>>>> {
		using ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<ReqRes::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>::ServerSocketSpec;
	};
}

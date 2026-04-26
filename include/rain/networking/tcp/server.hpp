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
			WorkerSocketSpec>,
		virtual public ServerSocketSpecInterfaceInterface {
		using Socket::Socket;
	};

	// Shorthand for TCP Server.
	template<
		typename WorkerSocketSpec,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template<typename> class... SocketOptions>
	class Server :
		public ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Networking::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>> {
		using ServerSocketSpec<
			WorkerSocketSpec,
			NamedSocketSpec<SocketSpec<Networking::Server<
				WorkerSocketSpec,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>::ServerSocketSpec;
	};
}

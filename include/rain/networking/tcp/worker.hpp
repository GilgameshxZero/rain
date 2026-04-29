// Worker specialization for TCP protocol Sockets.
#pragma once

#include "../worker.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	class WorkerSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public Networking::
			WorkerSocketSpecInterfaceInterface {};

	class WorkerSocketSpecInterface :
		virtual public WorkerSocketSpecInterfaceInterface,
		virtual public Networking::WorkerSocketSpecInterface {};

	// Worker specialization for TCP protocol Sockets.
	template<typename Socket>
	class WorkerSocketSpec :
		public Socket,
		virtual public WorkerSocketSpecInterface {
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
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Worker :
		public WorkerSocketSpec<ConnectedSocketSpec<
			NamedSocketSpec<SocketSpec<Networking::Worker<
				SocketFamilyInterface,
				StreamTypeInterface,
				TcpProtocolInterface,
				SocketOptions...>>>>> {
		public:
		using WorkerSocketSpec =
			WorkerSocketSpec<ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<Networking::Worker<
					SocketFamilyInterface,
					StreamTypeInterface,
					TcpProtocolInterface,
					SocketOptions...>>>>>;
		using WorkerSocketSpec::WorkerSocketSpec;
	};
}

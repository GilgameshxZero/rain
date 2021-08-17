// The R/R Socket protocol implements Workers/Clients where Clients send
// Requests and Workers send Responses.
//
// The R/R Socket protocol is abstract and cannot be instantiated directly.
// Thus, with the exception of Socket, only interface implementations are
// provided.
#pragma once

#include "../tcp/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::ReqRes {
	// The R/R Socket protocol implements Workers/Clients where Clients send
	// Requests and Workers send Responses.
	class SocketSpecInterface : virtual public Tcp::SocketSpecInterface {};

	template <typename Socket>
	class SocketSpec : public Socket, virtual public SocketSpecInterface {
		using Socket::Socket;
	};

	class NamedSocketSpecInterface
			: virtual public SocketSpecInterface,
				virtual public Tcp::NamedSocketSpecInterface {};

	template <typename Socket>
	class NamedSocketSpec : public Socket,
													virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	// Allows for send/recv on R/Rs.
	class ConnectedSocketSpecInterface
			: virtual public NamedSocketSpecInterface,
				virtual public Tcp::ConnectedSocketSpecInterface {};

	template <typename Socket>
	class ConnectedSocketSpec : public Socket,
															virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;
	};
}

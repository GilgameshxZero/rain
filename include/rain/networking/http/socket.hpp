// HTTP Socket subclassing R/R Socket.
#pragma once

#include "../req-res/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Http {
	// HTTP Socket subclassing R/R Socket.
	class SocketSpecInterface : virtual public ReqRes::SocketSpecInterface {
		protected:
		// Import Http namespace names for subclasses declared outside
		// Rain::Networking::Http.
		using StatusCode = StatusCode;
		using Method = Method;
		using Headers = Headers;
		using Body = Body;
		using Version = Version;
		using MediaType = MediaType;
	};

	template <typename Socket>
	class SocketSpec : public Socket, virtual public SocketSpecInterface {
		using Socket::Socket;
	};

	class NamedSocketSpecInterface
			: virtual public SocketSpecInterface,
				virtual public ReqRes::NamedSocketSpecInterface {};

	template <typename Socket>
	class NamedSocketSpec : public Socket,
													virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	class ConnectedSocketSpecInterface
			: virtual public NamedSocketSpecInterface,
				virtual public ReqRes::ConnectedSocketSpecInterface {};

	template <typename Socket>
	class ConnectedSocketSpec : public Socket,
															virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;
	};
}

// SMTP Socket subclassing R/R Socket.
#pragma once

#include "../req-res/socket.hpp"
#include "auth-method.hpp"
#include "mailbox.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Smtp {
	// SMTP Socket subclassing R/R Socket.
	class SocketSpecInterface : virtual public ReqRes::SocketSpecInterface {
		protected:
		// Import Smtp namespace names for subclasses.
		using AuthMethod = AuthMethod;
		using Command = Command;
		using Mailbox = Mailbox;
		using StatusCode = StatusCode;
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

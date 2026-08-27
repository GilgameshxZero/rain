// IMAP Socket subclassing R/R Socket.
#pragma once

#include "../req_res/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Imap {
	// IMAP Socket subclassing R/R Socket.
	class SocketSpecInterface :
		virtual public ReqRes::SocketSpecInterface {
	};

	template<typename Socket>
	class SocketSpec :
		public Socket,
		virtual public SocketSpecInterface {
		using Socket::Socket;
	};

	class NamedSocketSpecInterface :
		virtual public SocketSpecInterface,
		virtual public ReqRes::NamedSocketSpecInterface {
		public:
		using SocketSpecInterface = Imap::SocketSpecInterface;
	};

	template<typename Socket>
	class NamedSocketSpec :
		public Socket,
		virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	class ConnectedSocketSpecInterface :
		virtual public NamedSocketSpecInterface,
		virtual public ReqRes::ConnectedSocketSpecInterface {
		public:
		using NamedSocketSpecInterface =
			Imap::NamedSocketSpecInterface;
	};

	template<typename Socket>
	class ConnectedSocketSpec :
		public Socket,
		virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;
	};
}

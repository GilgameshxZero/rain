// Protocol layer TLS *wraps* an existing socket. Some
// connections are opportunistic TLS; they begin in
// plaintext (often over another protocol, such as SMTP),
// and only begin the TLS handshake admist an existing
// connection. A TLS socket should support both methods of
// construction: via wrapping (move constructing) from an
// existing socket, or constructing and connecting a new
// one.
//
// Since TLS works with multiple underlying protocols, a
// template parameter must be provided for the underlying
// protocol.
#pragma once

namespace Rain::Networking::Tls {
	template<typename UnderlyingSocketSpecInterface>
	class SocketSpecInterface :
		virtual public UnderlyingSocketSpecInterface {};

	template<
		typename UnderlyingSocketSpecInterface,
		typename Socket>
	class SocketSpec :
		public Socket,
		virtual public SocketSpecInterface<
			UnderlyingSocketSpecInterface> {
		using Socket::Socket;
	};

	template<typename UnderlyingNamedSocketSpecInterface>
	class NamedSocketSpecInterface :
		virtual public SocketSpecInterface<
			typename UnderlyingNamedSocketSpecInterface::
				SocketSpecInterface>,
		virtual public UnderlyingNamedSocketSpecInterface {
		public:
		using SocketSpecInterface = Tls::SocketSpecInterface<
			typename UnderlyingNamedSocketSpecInterface::
				SocketSpecInterface>;
	};

	template<
		typename UnderlyingNamedSocketSpecInterface,
		typename Socket>
	class NamedSocketSpec :
		public Socket,
		virtual public NamedSocketSpecInterface<
			UnderlyingNamedSocketSpecInterface> {
		using Socket::Socket;
	};

	template<typename UnderlyingConnectedSocketSpecInterface>
	class ConnectedSocketSpecInterface :
		virtual public NamedSocketSpecInterface<
			typename UnderlyingConnectedSocketSpecInterface::
				NamedSocketSpecInterface>,
		virtual public UnderlyingConnectedSocketSpecInterface {
		public:
		using NamedSocketSpecInterface =
			Tls::NamedSocketSpecInterface<
				typename UnderlyingConnectedSocketSpecInterface::
					NamedSocketSpecInterface>;
	};

	template<
		typename UnderlyingConnectedSocketSpecInterface,
		typename Socket>
	class ConnectedSocketSpec :
		public Socket,
		virtual public ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface> {
		using Socket::Socket;
	};
}

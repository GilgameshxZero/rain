// TLS Client specialization.
#pragma once

#include "socket.hpp"

namespace Rain::Networking::Tls {
	template<
		typename UnderlyingClientSocketSpecInterfaceInterface>
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface<
			typename UnderlyingClientSocketSpecInterfaceInterface::
				ConnectedSocketSpecInterface>,
		virtual public UnderlyingClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			Tls::ConnectedSocketSpecInterface<
				typename UnderlyingClientSocketSpecInterfaceInterface::
					ConnectedSocketSpecInterface>;
	};

	template<typename UnderlyingClientSocketSpecInterface>
	class ClientSocketSpecInterface :
		virtual public ClientSocketSpecInterfaceInterface<
			typename UnderlyingClientSocketSpecInterface::
				ClientSocketSpecInterfaceInterface>,
		virtual public UnderlyingClientSocketSpecInterface {
		public:
		using ClientSocketSpecInterfaceInterface =
			ClientSocketSpecInterfaceInterface<
				typename UnderlyingClientSocketSpecInterface::
					ClientSocketSpecInterfaceInterface>;
	};

	template<
		typename UnderlyingClientSocketSpecInterface,
		typename Socket>
	class ClientSocketSpec :
		public Socket,
		virtual public ClientSocketSpecInterface<
			UnderlyingClientSocketSpecInterface>,
		virtual public ClientSocketSpecInterfaceInterface<
			typename UnderlyingClientSocketSpecInterface::
				ClientSocketSpecInterfaceInterface> {
		using Socket::Socket;
	};

	template<
		typename UnderlyingClientSocketSpecInterface,
		typename UnderlyingClientSocketSpec>
	class Client :
		public ClientSocketSpec<
			UnderlyingClientSocketSpecInterface,
			ConnectedSocketSpec<
				typename UnderlyingClientSocketSpecInterface::
					ConnectedSocketSpecInterface,
				NamedSocketSpec<
					typename UnderlyingClientSocketSpecInterface::
						ConnectedSocketSpecInterface::
							NamedSocketSpecInterface,
					SocketSpec<
						typename UnderlyingClientSocketSpecInterface::
							ConnectedSocketSpecInterface::
								NamedSocketSpecInterface::
									SocketSpecInterface,
						UnderlyingClientSocketSpec>>>> {
		using ClientSocketSpec<
			UnderlyingClientSocketSpecInterface,
			ConnectedSocketSpec<
				typename UnderlyingClientSocketSpecInterface::
					ConnectedSocketSpecInterface,
				NamedSocketSpec<
					typename UnderlyingClientSocketSpecInterface::
						ConnectedSocketSpecInterface::
							NamedSocketSpecInterface,
					SocketSpec<
						typename UnderlyingClientSocketSpecInterface::
							ConnectedSocketSpecInterface::
								NamedSocketSpecInterface::
									SocketSpecInterface,
						UnderlyingClientSocketSpec>>>>::
			ClientSocketSpec;

		public:
		// TODO: Add constructor to wrap an existing
		// UnderlyingClientSocketSpec.
	};
}

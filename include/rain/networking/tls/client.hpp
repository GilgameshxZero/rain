// TLS Client specialization.
#pragma once

#include "socket.hpp"

#include <iostream>
#include <utility>

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

	template<typename Socket>
	class ClientSocketSpec :
		public Socket,
		virtual public ClientSocketSpecInterface<
			typename Socket::ClientSocketSpecInterface> {
		using Socket::Socket;

		public:
		// This works because each protocol layer's
		// resource-managing (client) socket defines a
		// ClientSocketSpecInterface, which shadows the previous
		// protocol layer's resource-managing (client) socket's
		// definition.
		using ClientSocketSpecInterface =
			Tls::ClientSocketSpecInterface<
				typename Socket::ClientSocketSpecInterface>;

		// Upgrade an existing client socket into a TLS client
		// socket is allowed.
		//
		// The previous socket is moved away from, and must not
		// be interacted with until its destruction.
		//
		// TODO: If we had allowed and implemented move
		// constructors (which are not inherited by `using`) all
		// across the stack, then this becomes easier to
		// implement here. Instead, since we don't have move
		// constructors, we must call `swap` here. This also
		// means we lose any state defined in the middle of the
		// stack (i.e. all state in protocol layers beneath this
		// and above Networking::Socket).
		ClientSocketSpec(
			typename Socket::ClientSocketSpec
				&&underlyingSocket) :
			Socket() {
			Socket::swap(&underlyingSocket);
		}
	};

	// Shorthand.
	template<typename UnderlyingClientSocketSpec>
	class Client :
		public ClientSocketSpec<ConnectedSocketSpec<
			typename UnderlyingClientSocketSpec::
				ClientSocketSpecInterface::
					ConnectedSocketSpecInterface,
			NamedSocketSpec<
				typename UnderlyingClientSocketSpec::
					ClientSocketSpecInterface::
						ConnectedSocketSpecInterface::
							NamedSocketSpecInterface,
				SocketSpec<
					typename UnderlyingClientSocketSpec::
						ClientSocketSpecInterface::
							ConnectedSocketSpecInterface::
								NamedSocketSpecInterface::
									SocketSpecInterface,
					UnderlyingClientSocketSpec>>>> {
		public:
		using ClientSocketSpec =
			ClientSocketSpec<ConnectedSocketSpec<
				typename UnderlyingClientSocketSpec::
					ClientSocketSpecInterface::
						ConnectedSocketSpecInterface,
				NamedSocketSpec<
					typename UnderlyingClientSocketSpec::
						ClientSocketSpecInterface::
							ConnectedSocketSpecInterface::
								NamedSocketSpecInterface,
					SocketSpec<
						typename UnderlyingClientSocketSpec::
							ClientSocketSpecInterface::
								ConnectedSocketSpecInterface::
									NamedSocketSpecInterface::
										SocketSpecInterface,
						UnderlyingClientSocketSpec>>>>;
		using ClientSocketSpec::ClientSocketSpec;
	};
}

// HTTP Client specialization.
#pragma once

#include "../req-res/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	class ClientSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public ReqRes::ClientSocketSpecInterfaceInterface {};

	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class ClientSocketSpecInterface
			: virtual public ClientSocketSpecInterfaceInterface,
				virtual public ReqRes::
					ClientSocketSpecInterface<RequestMessageSpec, ResponseMessageSpec> {};

	// HTTP Client specialization.
	template <
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename Socket>
	class ClientSocketSpec : public Socket,
													 virtual public ClientSocketSpecInterface<
														 RequestMessageSpec,
														 ResponseMessageSpec>,
													 virtual public ClientSocketSpecInterfaceInterface {
		using Socket::Socket;
	};

	// Shorthand.
	template <
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		std::size_t sendBufferLen,
		std::size_t recvBufferLen,
		long long sendTimeoutMs,
		long long recvTimeoutMs,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Client
			: public ClientSocketSpec<
					RequestMessageSpec,
					ResponseMessageSpec,
					ConnectedSocketSpec<NamedSocketSpec<SocketSpec<ReqRes::Client<
						RequestMessageSpec,
						ResponseMessageSpec,
						sendBufferLen,
						recvBufferLen,
						sendTimeoutMs,
						recvTimeoutMs,
						SocketFamilyInterface,
						SocketTypeInterface,
						SocketProtocolInterface,
						SocketOptions...>>>>> {
		using ClientSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<NamedSocketSpec<SocketSpec<ReqRes::Client<
				RequestMessageSpec,
				ResponseMessageSpec,
				sendBufferLen,
				recvBufferLen,
				sendTimeoutMs,
				recvTimeoutMs,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>>::ClientSocketSpec;
	};
}

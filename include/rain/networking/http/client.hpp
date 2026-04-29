// HTTP Client specialization.
#pragma once

#include "../req_res/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public ReqRes::
			ClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			Http::ConnectedSocketSpecInterface;
	};

	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec>
	class ClientSocketSpecInterface :
		virtual public ClientSocketSpecInterfaceInterface,
		virtual public ReqRes::ClientSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec> {
		public:
		using ClientSocketSpecInterfaceInterface =
			Http::ClientSocketSpecInterfaceInterface;
	};

	// HTTP Client specialization.
	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename Socket>
	class ClientSocketSpec :
		public Socket,
		virtual public ClientSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec> {
		using Socket::Socket;

		public:
		using ClientSocketSpecInterface =
			Http::ClientSocketSpecInterface<
				RequestMessageSpec,
				ResponseMessageSpec>;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	template<
		typename RequestMessageSpec = Http::Request,
		typename ResponseMessageSpec = Http::Response,
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Client :
		public ClientSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<ReqRes::Client<
					RequestMessageSpec,
					ResponseMessageSpec,
					SocketFamilyInterface,
					SocketOptions...>>>>> {
		public:
		using ClientSocketSpec = ClientSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<ReqRes::Client<
					RequestMessageSpec,
					ResponseMessageSpec,
					SocketFamilyInterface,
					SocketOptions...>>>>>;
		using ClientSocketSpec::ClientSocketSpec;
	};
}

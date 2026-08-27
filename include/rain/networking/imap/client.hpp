#pragma once

#include "../req_res/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Imap {
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public ReqRes::
			ClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			Imap::ConnectedSocketSpecInterface;
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
			Imap::ClientSocketSpecInterfaceInterface;
	};

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
			Imap::ClientSocketSpecInterface<
				RequestMessageSpec,
				ResponseMessageSpec>;

		using Request = RequestMessageSpec;
		using Response = ResponseMessageSpec;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	template<
		typename RequestMessageSpec = Imap::Request,
		typename ResponseMessageSpec = Imap::Response,
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

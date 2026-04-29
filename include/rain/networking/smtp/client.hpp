// SMTP Client specialization.
#pragma once

#include "../req_res/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public ReqRes::
			ClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			Smtp::ConnectedSocketSpecInterface;

		protected:
		// Resolve MX records into addresses.
		static std::vector<Host> mxRecordsToHostGroups(
			std::vector<std::pair<std::size_t, std::string>> const
				&mxRecords,
			std::size_t port = 25) {
			std::vector<Host> hostGroups;
			for (auto const &mxRecord : mxRecords) {
				hostGroups.emplace_back(mxRecord.second, port);
			}
			return hostGroups;
		}
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
			Smtp::ClientSocketSpecInterfaceInterface;
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
			Smtp::ClientSocketSpecInterface<
				RequestMessageSpec,
				ResponseMessageSpec>;

		using Request = RequestMessageSpec;
		using Response = ResponseMessageSpec;

		// Provides an additional constructor for the records
		// returned from an MX lookup.
		ClientSocketSpec(
			std::vector<std::pair<std::size_t, std::string>> const
				&mxRecords,
			std::size_t port = 25,
			Time::Timeout timeout = 15s,
			AddressInfo::Flag flags =
				AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG |
				AddressInfo::Flag::ALL) :
			Socket(
				ClientSocketSpecInterfaceInterface::
					mxRecordsToHostGroups(mxRecords, port),
				timeout,
				flags) {}
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	template<
		typename RequestMessageSpec = Smtp::Request,
		typename ResponseMessageSpec = Smtp::Response,
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

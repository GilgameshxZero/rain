// SMTP Client specialization.
#pragma once

#include "../req-res/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	class ClientSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public ReqRes::ClientSocketSpecInterfaceInterface {
		protected:
		// Resolve MX records into addresses.
		static std::vector<Host> mxRecordsToHostGroups(
			std::vector<std::pair<std::size_t, std::string>> const &mxRecords,
			std::size_t port = 25) {
			std::vector<Host> hostGroups;
			for (auto const &mxRecord : mxRecords) {
				hostGroups.emplace_back(mxRecord.second, port);
			}
			return hostGroups;
		}
	};

	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class ClientSocketSpecInterface
			: virtual public ClientSocketSpecInterfaceInterface,
				virtual public ReqRes::
					ClientSocketSpecInterface<RequestMessageSpec, ResponseMessageSpec> {};

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

		public:
		using Request = RequestMessageSpec;
		using Response = ResponseMessageSpec;

		// Provides an additional constructor for the records returned from an MX
		// lookup.
		ClientSocketSpec(
			std::vector<std::pair<std::size_t, std::string>> const &mxRecords,
			std::size_t port = 25,
			Time::Timeout timeout = 15s,
			AddressInfo::Flag flags = AddressInfo::Flag::V4MAPPED |
				AddressInfo::Flag::ADDRCONFIG | AddressInfo::Flag::ALL)
				: Socket(
						ClientSocketSpecInterfaceInterface::mxRecordsToHostGroups(
							mxRecords,
							port),
						timeout,
						flags) {}
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

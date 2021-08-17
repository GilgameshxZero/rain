// Client specialization for R/R sockets.
#pragma once

#include "../tcp/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::ReqRes {
	class ClientSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public Tcp::ClientSocketSpecInterface {};

	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class ClientSocketSpecInterface
			: virtual public ClientSocketSpecInterfaceInterface {
		public:
		using ClientSocketSpecInterfaceInterface::send;
		using ClientSocketSpecInterfaceInterface::recv;
		using ClientSocketSpecInterfaceInterface::operator<<;
		using ClientSocketSpecInterfaceInterface::operator>>;

		// Always completes with success, or throws or sets the stream into a bad
		// state on failure.
		//
		// Override here for pp-chaining, but limited to send/recv on Sockets.
		virtual void send(RequestMessageSpec &req) { req.sendWith(*this); }
		virtual ResponseMessageSpec &recv(ResponseMessageSpec &res) {
			res.recvWith(*this);
			return res;
		}

		// For inline construction.
		void send(RequestMessageSpec &&req) { this->send(req); }
		ResponseMessageSpec recv() {
			ResponseMessageSpec res;
			this->recv(res);
			return res;
		}

		// std::iostream overloads.
		ConnectedSocketSpecInterface &operator<<(RequestMessageSpec &req) {
			this->send(req);
			return *this;
		}
		ConnectedSocketSpecInterface &operator>>(ResponseMessageSpec &res) {
			this->recv(res);
			return *this;
		}

		// For inline construction.
		ConnectedSocketSpecInterface &operator<<(RequestMessageSpec &&req) {
			return *this << req;
		}
	};

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

	// Shorthand for R/R Client.
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
	class Client : public ClientSocketSpec<
									 RequestMessageSpec,
									 ResponseMessageSpec,
									 ConnectedSocketSpec<NamedSocketSpec<SocketSpec<Tcp::Client<
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
			ConnectedSocketSpec<NamedSocketSpec<SocketSpec<Tcp::Client<
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

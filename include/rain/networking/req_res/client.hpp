// Client specialization for R/R sockets.
#pragma once

#include "../tcp/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::ReqRes {
	class ClientSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public Tcp::ClientSocketSpecInterfaceInterface {
		public:
		using ConnectedSocketSpecInterface =
			ReqRes::ConnectedSocketSpecInterface;
	};

	// All R/R *SpecInterfaces accept R/R typename parameters,
	// since they are meant to be decided by the caller
	// (likely a protocol layer atop R/R, e.g. HTTP/SMTP).
	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec>
	class ClientSocketSpecInterface :
		virtual public ClientSocketSpecInterfaceInterface,
		virtual public Tcp::ClientSocketSpecInterface {
		public:
		using ClientSocketSpecInterfaceInterface =
			ReqRes::ClientSocketSpecInterfaceInterface;

		using ClientSocketSpecInterfaceInterface::recv;
		using ClientSocketSpecInterfaceInterface::send;
		using ClientSocketSpecInterfaceInterface::operator<<;
		using ClientSocketSpecInterfaceInterface::operator>>;

		// Always completes with success, or throws or sets the
		// stream into a bad state on failure.
		//
		// Override here for pp-chaining, but limited to
		// send/recv on Sockets.
		virtual void send(RequestMessageSpec &req) {
			req.sendWith(*this);
		}
		virtual ResponseMessageSpec &recv(
			ResponseMessageSpec &res) {
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
		ConnectedSocketSpecInterface &operator<<(
			RequestMessageSpec &req) {
			this->send(req);
			return *this;
		}
		ConnectedSocketSpecInterface &operator>>(
			ResponseMessageSpec &res) {
			this->recv(res);
			return *this;
		}

		// For inline construction.
		ConnectedSocketSpecInterface &operator<<(
			RequestMessageSpec &&req) {
			return *this << req;
		}
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
			ReqRes::ClientSocketSpecInterface<
				RequestMessageSpec,
				ResponseMessageSpec>;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	//
	// R/R do not get default template arguments (as the only
	// relevant ones are abstract in R/R), but protocol layers
	// atop R/R may specify default template arguments.
	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Client :
		public ClientSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<Tcp::Client<
					SocketFamilyInterface,
					SocketOptions...>>>>> {
		public:
		using ClientSocketSpec = ClientSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<Tcp::Client<
					SocketFamilyInterface,
					SocketOptions...>>>>>;
		using ClientSocketSpec::ClientSocketSpec;
	};
}

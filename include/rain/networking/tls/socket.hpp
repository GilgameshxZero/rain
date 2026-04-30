// Protocol layer TLS *wraps* an existing socket. Some
// connections are opportunistic TLS; they begin in
// plaintext (often over another protocol, such as SMTP),
// and only begin the TLS handshake amidst an existing
// connection. A TLS socket should support both methods of
// construction: via wrapping (move constructing) from an
// existing socket, or constructing and connecting a new
// one.
//
// Since TLS works with multiple underlying protocols, a
// template parameter must be provided for the underlying
// protocol.
#pragma once

#include "plaintext.hpp"

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

		using UnderlyingConnectedSocketSpecInterface::send;
		using UnderlyingConnectedSocketSpecInterface::recv;
		using UnderlyingConnectedSocketSpecInterface::
			operator<<;
		using UnderlyingConnectedSocketSpecInterface::operator>>
			;

		// Overloads to send TLS types.
		//
		// Again, since TLS types may be ephemeral (changed by
		// the send), we must provide send in both
		// non-const-lvalue-reference and rvalue-reference
		// forms.
		//
		// Since member function templates cannot be virtual,
		// this makes it difficult to PP-chain like in R/R
		// sockets. Instead, PP-chain by overloading
		// Plaintext/*'s sendWith/recvWith.
		//
		// Since Plaintext/* are not template parameters (as
		// opposed to the case of R/R), but rather fixed types,
		// this makes more sense to do here.
		template<
			typename Message,
			decltype(std::declval<Message>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		void send(Message &message) {
			message.sendWith(*this);
		}
		template<
			typename Message,
			decltype(std::declval<Message>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		Message &recv(Message &message) {
			message.recvWith(*this);
			return message;
		}
		// For inline construction.
		template<
			typename Message,
			decltype(std::declval<Message>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		void send(Message &&message) {
			this->send(message);
		}
		template<
			typename Message,
			decltype(std::declval<Message>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		Message recv() {
			Message message;
			this->recv(message);
			return message;
		}

		// std::iostream overloads.
		template<
			typename Message,
			decltype(std::declval<Message>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator<<(
			Message &message) {
			this->send(message);
			return *this;
		}
		template<
			typename Message,
			decltype(std::declval<Message>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator>>(
			Message &message) {
			this->recv(message);
			return *this;
		}
		// For inline construction.
		template<
			typename Message,
			decltype(std::declval<Message>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator<<(
			Message &&message) {
			return *this << message;
		}
	};

	template<
		typename UnderlyingConnectedSocketSpecInterface,
		typename Socket>
	class ConnectedSocketSpec :
		public Socket,
		virtual public ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface> {
		using Socket::Socket;

		public:
		using ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface>::send;
		using ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface>::recv;
		using ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface>::operator<<;
		using ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface>::operator>>;
	};
}

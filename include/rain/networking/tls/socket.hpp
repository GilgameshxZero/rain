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
		//
		// recvWith functions are largely unused because TLS
		// record types are usually valid upon construction.
		template<
			typename Record,
			decltype(std::declval<Record>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		void send(Record &record) {
			record.sendWith(*this);
		}
		template<
			typename Record,
			decltype(std::declval<Record>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		Record &recv(Record &record) {
			record.recvWith(*this);
			return record;
		}
		// For inline construction.
		template<
			typename Record,
			decltype(std::declval<Record>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		void send(Record &&record) {
			this->send(record);
		}
		template<
			typename Record,
			decltype(std::declval<Record>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		Record recv() {
			Record record;
			this->recv(record);
			return record;
		}

		// std::iostream overloads.
		template<
			typename Record,
			decltype(std::declval<Record>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator<<(
			Record &record) {
			this->send(record);
			return *this;
		}
		template<
			typename Record,
			decltype(std::declval<Record>().recvWith(
				std::declval<std::istream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator>>(
			Record &record) {
			this->recv(record);
			return *this;
		}
		// For inline construction.
		template<
			typename Record,
			decltype(std::declval<Record>().sendWith(
				std::declval<std::ostream &>())) * = nullptr>
		ConnectedSocketSpecInterface &operator<<(
			Record &&record) {
			return *this << record;
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

		// TODO: Add streambufs for each content type.
		// streambufs should read from the common TCP streambuf
		// and use common Plaintext/Ciphertext unwrapping
		// functions. streambufs should write to a
		// current-configuration Plaintext/Ciphertext before
		// pushing to the TCP streambuf.
		//
		// TODO: streambuf parameters should probably be put
		// into a parameter pack.
	};
}

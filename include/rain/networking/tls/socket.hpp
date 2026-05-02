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

#include "content_interface.hpp"
#include "record.hpp"
#include "security_parameters.hpp"

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

		// The security parameters determines the type of Record
		// which is sent/received. They are also used by the
		// streambuffer to encode/decode any fragments.
		SecurityParameters securityParameters;

		// Custom streambuffer has shared static buffers between
		// all instances. Underflow reads from underlying TCP
		// stream and puts it into the right buffer, until the
		// requested buffer is filled.
		//
		// Overflow packs the buffer as a Record. The buffer
		// contents are first...
		//
		// TODO.
		class TlsStreamBuf : public std::streambuf {
			private:
			// Return a pair of send/recv buffers associated with
			// a specific content type. All TlsStreamBuf buffers
			// are static and accessible from any streambuf
			// instance.
			//
			// Buffer size is up to 4K (+ 2K for ciphertext) as
			// per RFC 5246. Total buffer size per socket is thus
			// up to 10K * |ContentType| = 40K.
			template<ContentType CONTENT_TYPE>
			std::pair<char *, char *> getBuffers() {
				char sendBuffer[4096 + 1], recvBuffer[4096 + 2048];
				return {sendBuffer, recvBuffer};
			}

			public:
			ContentType const CONTENT_TYPE;

			// Must be constructed with the content type to
			// identify the underlying buffer. However, all
			// buffers are accessible to any streambuf instance,
			// because one streambuff may fill another's buffer
			// during underflow.
			TlsStreamBuf(ContentType const &contentType) :
				CONTENT_TYPE{contentType} {}

			// Copy/move is implicitly disabed because of const
			// TYPE.

			protected:
			// recvBuffer needs to be refilled.
			virtual int_type underflow() override { return 0; }

			// sendBuffer needs to be sent as a Record and
			// cleared.
			virtual int_type overflow(
				int_type = traits_type::eof()) override {
				return 0;
			}

			// Send sendbuffer as a Record.
			virtual int sync() override { return 0; };
		};

		// Retrieves a stream associated with a specific content
		// type.
		template<ContentType CONTENT_TYPE>
		std::iostream &getMessageStream() const {
			static std::unique_ptr<TlsStreamBuf> tlsStreamBuf(
				new TlsStreamBuf(CONTENT_TYPE));
			static std::iostream stream(tlsStreamBuf.get());
			return stream;
		}

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

		// TODO: Add buffers for each content type.
		// Buffer should read from the common TCP streambuf
		// and use common Plaintext/Ciphertext unwrapping
		// functions. When a buffer detects that it has a
		// complete Content (Alert/Handshake/*), it will push it
		// out onto the a content queue.
		//
		// TODO: streambuf parameters should probably be put
		// into a parameter pack.
	};
}

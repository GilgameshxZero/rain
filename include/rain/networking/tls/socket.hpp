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

#include "../../literal.hpp"
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
		// public std::iostream,
		virtual public NamedSocketSpecInterface<
			typename UnderlyingConnectedSocketSpecInterface::
				NamedSocketSpecInterface>,
		virtual public UnderlyingConnectedSocketSpecInterface {
		public:
		using NamedSocketSpecInterface =
			Tls::NamedSocketSpecInterface<
				typename UnderlyingConnectedSocketSpecInterface::
					NamedSocketSpecInterface>;

		using This = ConnectedSocketSpecInterface<
			UnderlyingConnectedSocketSpecInterface>;

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

		// Maintain internal buffers for each content type.
		// Since a changeCipherSpec message is always 1 byte,
		// and an alert message is always 2 bytes, we only
		// maintain meaningful buffers for handshake and
		// application data.
		//
		// Since the caller only interfaces directly with
		// application data, we only maintain a send buffer for
		// it, and not the other message types, which we mangage
		// internally at-send.
		//
		// The application data buffers are used with an
		// internal streambuf to supply the iostream superclass.
		// The iostream superclass must be distinct from any
		// underlying TCP iostream superclass, since they
		// maintain separate buffers. The current iostream
		// superclass should shadow any TCP iostream superclass
		// in all meaningful ways.
		//
		// This means that any TCP socket maintains at least 16M
		// in memory state.
		//
		// TODO: This seems high. Can we fragment received handshake processessing, somehow?
		private:
		// char recvBufferHandshake[1_zu << 24_zu],
		// 	sendBufferApplicationData[1_zu << 10_zu],
		// 	recvBufferApplicationData[1_zu << 10_zu];

		// Maintain the target length of a handshake message
		// so that a full message can be detected.
		std::size_t recvHandshakeTargetLength{};

		protected:
		// The onMessage() callback is called when a message
		// of any content type is fully available. It will
		// never be called concurrently. The message must be
		// fully processed within the callback as it will be
		// unavailable afterwards.
		//
		// Any-length application data will be considered a
		// "full" message and trigger onMessage().
		// virtual void onMessage(Message message) {}
		virtual void onMessage() {}

		private:
		// The default behavior handles the TCP handshake and
		// places any application data in the class istream
		// buffer. Underflowing the istream buffer before the
		// handshake is in a completed state will error.
		//
		// The class ostream buffer always sends as
		// application data. Overflowing the ostream buffer
		// before the handshake is in a completed state will
		// error.
		class TlsApplicationDataStreamBuf : std::streambuf {};

		public:
		// The class constructor will attempt to complete the
		// handshake. Thus, it should be impossible to
		// overflow/underflow into an error state, unless the
		// socket disconnects.
		ConnectedSocketSpecInterface() {}

		// blockForMessage() blocks until a full message is
		// available, and finished processing via onMessage().
		void blockForMessage() {}

		// private:
		// // Fragments of the same content type share the same
		// // buffers.
		// static inline constexpr std::size_t const
		// BUFFER_SIZE{ 	1_zu << 12_zu}; std::array<
		// 	std::array<char, This::BUFFER_SIZE>,
		// 	ContentType::_SIZE>
		// 	sendBuffers;
		// std::array<
		// 	std::array<char, This::BUFFER_SIZE>,
		// 	ContentType::_SIZE>
		// 	recvBuffers;

		// // Custom streambuffer has shared static buffers
		// between
		// // all instances. Underflow reads from underlying TCP
		// // stream and puts it into the right buffer, until
		// the
		// // requested buffer is filled.
		// //
		// // Underflow always reads an entire record from the
		// // socket, and decodes it into the buffer. Overflow
		// // always encodes the entire buffer into a record.
		// Both
		// // encode/decode depend on current
		// SecurityParameters. class TlsStreamBuf : public
		// std::streambuf { 	public:
		// 	// Overflow will send as this content type.
		// 	ContentType const CONTENT_TYPE;

		// 	// One send buffer corresponding to the current
		// 	// CONTENT_TYPE, and all recv buffers.
		// 	std::array<char, This::BUFFER_SIZE> &sendBuffer;
		// 	std::array<
		// 		std::array<char, This::BUFFER_SIZE>,
		// 		ContentType::_SIZE> &recvBuffers;

		// 	// Must be constructed with the content type to
		// 	// identify the underlying buffer. However, all
		// 	// buffers are accessible to any streambuf instance,
		// 	// because one streambuff may fill another's buffer
		// 	// during underflow.
		// 	TlsStreamBuf(
		// 		ContentType const &CONTENT_TYPE,
		// 		std::array<char, This::BUFFER_SIZE> &sendBuffer,
		// 		std::array<
		// 			std::array<char, This::BUFFER_SIZE>,
		// 			ContentType::_SIZE> &recvBuffers) :
		// 		CONTENT_TYPE{CONTENT_TYPE},
		// 		sendBuffer{sendBuffer},
		// 		recvBuffers{recvBuffers} {
		// 		// Set internal pointers corresponding to empty
		// 		// buffers.
		// 		this->setg(
		// 			this->recvBuffers[this->CONTENT_TYPE.asIndex()]
		// 				.data(),
		// 			this->recvBuffers[this->CONTENT_TYPE.asIndex()]
		// 				.data(),
		// 			this->recvBuffers[this->CONTENT_TYPE.asIndex()]
		// 				.data());
		// 		// Reserve final character in buffer for overflow.
		// 		this->setp(
		// 			this->sendBuffer.data(),
		// 			this->sendBuffer.data() + This::BUFFER_SIZE -
		// 1);
		// 	}

		// 	// Copy/move is implicitly disabled because of const
		// 	// CONTENT_TYPE and reference to buffers.

		// 	protected:
		// 	// sendBuffer needs to be sent as a Record and
		// 	// cleared.
		// 	virtual int_type overflow(
		// 		int_type ch = traits_type::eof()) override {
		// 		if (!traits_type::eq_int_type(
		// 					ch, traits_type::eof())) {
		// 			// Push ch to the send buffer (final character
		// in
		// 			// sendBuffer is reserved by us).
		// 			*this->pptr() = ch;
		// 			this->pbump(1);

		// 			// Flush and empty the send buffer.
		// 			if (this->sync() == -1) {
		// 				return traits_type::eof();
		// 			}
		// 		}

		// 		return ch;
		// 	}

		// 	// recvBuffer needs to be refilled.
		// 	virtual int_type underflow() override {
		// 		// Only refill buffer if it has been exhausted.
		// 		// if (this->gptr() == this->egptr()) {
		// 		// 	std::size_t result{};

		// 		// 	try {
		// 		// 		result = this->socket->recv(
		// 		// 			this->recvBuffer,
		// 		// 			this->RECV_BUFFER_LEN,
		// 		// 			std::chrono::milliseconds(
		// 		// 				this->RECV_TIMEOUT_MS));
		// 		// 	} catch (...) {
		// 		// 		// recv throws (perhaps on peer abort) are
		// 		// 		// consumed, setting the buffer as invalid
		// 		// 		// instead.
		// 		// 	}

		// 		// 	if (result == 0) {
		// 		// 		// The invariant this->gptr() ==
		// this->egptr()
		// 		// 		// is maintained.
		// 		// 		return traits_type::eof();
		// 		// 	}

		// 		// 	this->setg(
		// 		// 		this->recvBuffer,
		// 		// 		this->recvBuffer,
		// 		// 		this->recvBuffer + result);
		// 		// }

		// 		return traits_type::to_int_type(*this->gptr());
		// 	}

		// 	// Send sendbuffer as a Record.
		// 	virtual int sync() override { return 0; };
		// };

		// private:
		// // Streambufs and streams are kept private.
		// std::array<
		// 	std::unique_ptr<TlsStreamBuf>,
		// 	ContentType::_SIZE>
		// 	tlsStreamBufs;
		// std::array<
		// 	std::unique_ptr<std::iostream>,
		// 	ContentType::_SIZE>
		// 	messageStreams;

		// public:
		// // Interfaces must always have default constructors.
		// The
		// // default constructor late-initializes the
		// streambufs
		// // and streams, since they require iterating over the
		// // ContentType enum to construct.
		// ConnectedSocketSpecInterface() {
		// 	for (std::size_t i{}; i < ContentType::_SIZE; i++) {
		// 		this->tlsStreamBufs[i].reset(new TlsStreamBuf(
		// 			ContentType::_FROM_INDEX[i],
		// 			this->sendBuffers[i],
		// 			this->recvBuffers));
		// 		this->messageStreams[i].reset(
		// 			new
		// std::iostream(this->tlsStreamBufs[i].get()));
		// 	}
		// }

		// Disallow usage of underlying TCP buffer.

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

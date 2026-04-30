// Protocol layer Tcp builds upon the basic types of
// Socket(Interface), NamedSocket(Interface),
// ConnectedSocket(Interface), ClientSocket(Interface),
// WorkerSocket(Interface), and ServerSocket(Interface).
//
// Tcp does not add additional socket options nor F/T/P
// selectors.
#pragma once

#include "../../literal.hpp"
#include "../socket.hpp"

#include <iostream>

namespace Rain::Networking::Tcp {
	// TCP Sockets have the additional requirement that they
	// can only be applied to Type Stream and Protocol TCP.
	class SocketSpecInterface :
		virtual public Networking::SocketInterface,
		virtual public StreamTypeInterface,
		virtual public TcpProtocolInterface {};

	template<typename Socket>
	class SocketSpec :
		public Socket,
		virtual public SocketSpecInterface {
		using Socket::Socket;
	};

	class NamedSocketSpecInterface :
		virtual public SocketSpecInterface,
		virtual public Networking::NamedSocketSpecInterface {
		public:
		using SocketSpecInterface = Tcp::SocketSpecInterface;
	};

	template<typename Socket>
	class NamedSocketSpec :
		public Socket,
		virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	// ConnectedSocket(Interface) in TCP protocol layer must
	// provide subclassing for std::iostream.
	class ConnectedSocketSpecInterface :
		// While it seems wrong to inherit from iostream here,
		// this inheritance actually inherits no state, and
		// preserves the Interface status of this class. The
		// inheritance inherits operators and types, while state
		// is set in ConnectedSocketSpec via `rdbuf`.
		public std::iostream,
		virtual public NamedSocketSpecInterface,
		virtual public Networking::
			ConnectedSocketSpecInterface {
		public:
		using NamedSocketSpecInterface =
			Tcp::NamedSocketSpecInterface;

		// All interfaces must have default constructors to
		// allow for easy virtual inheritance.
		ConnectedSocketSpecInterface() :
			std::iostream(nullptr) {}
	};

	// Must instantiate a valid std::iostream and underlying
	// streambuf with params.
	//
	// Once recv times out once, the stream is set to an
	// invalid state and will no longer be readable. send
	// timeout functions similarly. However, the send timeout
	// is per-progress: as long as non-zero progress is made
	// per timeout period, send is not considered to have
	// timed out. Thus, flush may consume upwards of
	// (SEND_TIMEOUT_MS) * (SEND_BUFFER_LEN) time.
	template<typename Socket>
	class ConnectedSocketSpec :
		public Socket,
		virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;

		public:
		// Constants for the internal streambuf.
		std::size_t const SEND_BUFFER_LEN{1_zu << 10_zu},
			RECV_BUFFER_LEN{1_zu << 10_zu};
		long long const SEND_TIMEOUT_MS{15000LL},
			RECV_TIMEOUT_MS{15000LL};

		// Expose Socket::swap and not iostream::swap.
		using Socket::swap;

		private:
		// A custom subclass of std::streambuf as underlying the
		// std::iostream.
		class IoStreamBuf : public std::streambuf {
			private:
			std::size_t const SEND_BUFFER_LEN, RECV_BUFFER_LEN;
			long long const SEND_TIMEOUT_MS, RECV_TIMEOUT_MS;

			// A reference to the incomplete "underlying" socket.
			ConnectedSocketSpecInterface *socket;

			// Internal buffer to prevent delegating to kernel
			// send/recv too often. sendBuffer is actually 1
			// larger than specified to allow for easy overflow.
			char *sendBuffer, *recvBuffer;

			public:
			IoStreamBuf(
				std::size_t const SEND_BUFFER_LEN,
				std::size_t const RECV_BUFFER_LEN,
				long long const SEND_TIMEOUT_MS,
				long long const RECV_TIMEOUT_MS,
				ConnectedSocketSpecInterface *socket) :
				SEND_BUFFER_LEN{SEND_BUFFER_LEN},
				RECV_BUFFER_LEN{RECV_BUFFER_LEN},
				SEND_TIMEOUT_MS{SEND_TIMEOUT_MS},
				RECV_TIMEOUT_MS{RECV_TIMEOUT_MS},
				socket(socket) {
				this->sendBuffer =
					new char[this->SEND_BUFFER_LEN + 1];
				this->recvBuffer = new char[this->RECV_BUFFER_LEN];

				// Set internal pointers corresponding to empty
				// buffers.
				this->setg(
					this->recvBuffer,
					this->recvBuffer,
					this->recvBuffer);
				this->setp(
					this->sendBuffer,
					this->sendBuffer + this->SEND_BUFFER_LEN);
			}

			// Disable copy construct/assignment and move
			// construct/assignment because we hold state.
			IoStreamBuf(IoStreamBuf const &) = delete;
			IoStreamBuf &operator=(IoStreamBuf const &) = delete;
			IoStreamBuf(IoStreamBuf &&) = delete;
			IoStreamBuf &operator=(IoStreamBuf &&) = delete;

			// Free stack memory on destruct.
			~IoStreamBuf() {
				delete[] this->sendBuffer;
				delete[] this->recvBuffer;
			}

			protected:
			// Ran out of characters while receiving from the
			// socket.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					std::size_t result{0};

					try {
						result = this->socket->recv(
							this->recvBuffer,
							this->RECV_BUFFER_LEN,
							std::chrono::milliseconds(
								this->RECV_TIMEOUT_MS));
					} catch (...) {
						// recv throws (perhaps on peer abort) are
						// consumed, setting the buffer as invalid
						// instead.
					}

					if (result == 0) {
						// The invariant this->gptr() == this->egptr()
						// is maintained.
						return traits_type::eof();
					}

					this->setg(
						this->recvBuffer,
						this->recvBuffer,
						this->recvBuffer + result);
				}

				return traits_type::to_int_type(*this->gptr());
			}

			// Ran out of buffer space while sending to the
			// socket.
			virtual int_type overflow(
				int_type ch =
					traits_type::eof()) noexcept override {
				if (!traits_type::eq_int_type(
							ch, traits_type::eof())) {
					// Push ch to the send buffer (final character in
					// sendBuffer is reserved by us).
					*this->pptr() = ch;
					this->pbump(1);

					// Flush and empty the send buffer.
					if (this->sync() == -1) {
						return traits_type::eof();
					}
				}

				return ch;
			}

			// Write available buffer to the socket.
			virtual int sync() noexcept override {
				// Send available buffer.
				try {
					std::size_t bytesSent{0},
						bytesTotal{static_cast<std::size_t>(
							this->pptr() - this->pbase())};
					while (bytesSent < bytesTotal) {
						auto result = this->socket->send(
							this->sendBuffer + bytesSent,
							bytesTotal - bytesSent,
							std::chrono::milliseconds(
								this->SEND_TIMEOUT_MS));
						if (result == 0) {
							return -1;
						}
						bytesSent += result;
					}
				} catch (...) {
					// Failing on send will set failbit on the
					// wrapper iostream.
					return -1;
				}

				// Mark the send buffer as empty.
				this->setp(
					this->sendBuffer,
					this->sendBuffer + this->SEND_BUFFER_LEN);

				return 0;
			}
		};

		// Internally holds the stream buffer.
		IoStreamBuf ioStreamBuf{IoStreamBuf(
			this->SEND_BUFFER_LEN,
			this->RECV_BUFFER_LEN,
			this->SEND_TIMEOUT_MS,
			this->RECV_TIMEOUT_MS,
			this)};

		private:
		// Custom defaulted default constructor behavior to add
		// onto base class constructor behavior, with all
		// constructors.
		class _ConnectedSocketSpec {
			public:
			_ConnectedSocketSpec(ConnectedSocketSpec *that) {
				that->rdbuf(&that->ioStreamBuf);
			}
		};
		// Syntax necessary to differentiate from function.
		_ConnectedSocketSpec _connectedSocketSpec{this};

		public:
		// Curry constructor to set timeout values.
		//
		// Using a curry constructor requires a non-deducing (no
		// initializer lists) set of other arguments.
		//
		// Interfaces get initialize before the main socket, so
		// the constants must not live in the interface, but
		// rather in the main socket.
		ConnectedSocketSpec(
			std::size_t const SEND_BUFFER_LEN,
			std::size_t const RECV_BUFFER_LEN,
			long long const SEND_TIMEOUT_MS,
			long long const RECV_TIMEOUT_MS,
			auto &&...args) :
			Socket(std::forward<decltype(args)>(args)...),
			SEND_BUFFER_LEN{SEND_BUFFER_LEN},
			RECV_BUFFER_LEN{RECV_BUFFER_LEN},
			SEND_TIMEOUT_MS{SEND_TIMEOUT_MS},
			RECV_TIMEOUT_MS{RECV_TIMEOUT_MS} {}
	};
}

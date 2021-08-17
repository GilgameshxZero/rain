// Protocol layer Tcp builds upon the basic types of Socket(Interface),
// NamedSocket(Interface), ConnectedSocket(Interface), ClientSocket(Interface),
// WorkerSocket(Interface), and ServerSocket(Interface).
//
// Tcp does not add additional socket options nor F/T/P selectors.
#pragma once

#include "../socket.hpp"

#include <iostream>

namespace Rain::Networking::Tcp {
	// TCP Sockets have the additional requirement that they can only be applied
	// to Type Stream and Protocol TCP.
	class SocketSpecInterface : virtual public Networking::SocketInterface,
															virtual public StreamTypeInterface,
															virtual public TcpProtocolInterface {};

	template <typename Socket>
	class SocketSpec : public Socket, virtual public SocketSpecInterface {
		using Socket::Socket;
	};

	class NamedSocketSpecInterface
			: virtual public SocketSpecInterface,
				virtual public Networking::NamedSocketSpecInterface {};

	template <typename Socket>
	class NamedSocketSpec : public Socket,
													virtual public NamedSocketSpecInterface {
		using Socket::Socket;
	};

	// ConnectedSocket(Interface) in TCP protocol layer must provide subclassing
	// for std::iostream.
	class ConnectedSocketSpecInterface
			: public std::iostream,
				virtual public NamedSocketSpecInterface,
				virtual public Networking::ConnectedSocketSpecInterface {
		public:
		// All interfaces must have default constructors to allow for easy virtual
		// inheritance.
		ConnectedSocketSpecInterface() : std::iostream(nullptr) {}
	};

	// Must instantiate a valid std::iostream and underlying streambuf with
	// params.
	//
	// Once recv times out once, the stream is set to an invalid state and will no
	// longer be readable. send timeout functions similarly. However, the send
	// timeout is per-progress: as long as non-zero progress is made per timeout
	// period, send is not considered to have timed out. Thus, flush may consume
	// upwards of (send timeout) * (sendBufferLen) time.
	template <
		std::size_t sendBufferLen,
		std::size_t recvBufferLen,
		long long sendTimeoutMs,
		long long recvTimeoutMs,
		typename Socket>
	class ConnectedSocketSpec : public Socket,
															virtual public ConnectedSocketSpecInterface {
		using Socket::Socket;

		private:
		// A custom subclass of std::streambuf as underlying the std::iostream.
		class IoStreamBuffer : public std::streambuf {
			private:
			// A reference to the incomplete "underlying" socket.
			ConnectedSocketSpecInterface *socket;

			// Internal buffer to prevent delegating to kernel send/recv too often.
			// sendBuffer is actually 1 larger than specified to allow for easy
			// overflow.
			char sendBuffer[sendBufferLen + 1], recvBuffer[recvBufferLen];

			public:
			IoStreamBuffer(ConnectedSocketSpecInterface *socket) : socket(socket) {
				// Set internal pointers corresponding to empty buffers.
				this->setg(this->recvBuffer, this->recvBuffer, this->recvBuffer);
				this->setp(this->sendBuffer, this->sendBuffer + sendBufferLen);
			}

			// Disable copy construct and assignment and move.
			IoStreamBuffer(IoStreamBuffer const &) = delete;
			IoStreamBuffer &operator=(IoStreamBuffer const &) = delete;
			IoStreamBuffer(IoStreamBuffer &&) = delete;
			IoStreamBuffer &operator=(IoStreamBuffer &&) = delete;

			protected:
			// Ran out of characters while receiving from the socket.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					std::size_t result = 0;

					try {
						result = this->socket->recv(
							this->recvBuffer,
							recvBufferLen,
							std::chrono::milliseconds(recvTimeoutMs));
					} catch (...) {
						// recv throws (perhaps on peer abort) are consumed, setting the
						// buffer as invalid instead.
					}

					if (result == 0) {
						// The invariant this->gptr() == this->egptr() is maintained.
						return traits_type::eof();
					}

					this->setg(
						this->recvBuffer, this->recvBuffer, this->recvBuffer + result);
				}

				return traits_type::to_int_type(*this->gptr());
			}

			// Write available buffer to the socket.
			virtual int sync() noexcept override {
				// Send available buffer.
				try {
					std::size_t bytesSent = 0, bytesTotal = this->pptr() - this->pbase();
					while (bytesSent < bytesTotal) {
						auto result = this->socket->send(
							this->sendBuffer + bytesSent,
							bytesTotal - bytesSent,
							std::chrono::milliseconds(sendTimeoutMs));
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
				this->setp(this->sendBuffer, this->sendBuffer + sendBufferLen);

				return 0;
			}

			// Ran out of buffer space while sending to the socket.
			virtual int_type overflow(
				int_type ch = traits_type::eof()) noexcept override {
				if (ch != traits_type::eof()) {
					// Push ch to the send buffer (final character in sendBuffer is
					// reserved by us).
					*this->pptr() = ch;
					this->pbump(1);

					// Flush and empty the send buffer.
					if (this->sync() == -1) {
						return traits_type::eof();
					}
				}

				return ch;
			}
		};

		// Internally holds the stream buffer.
		IoStreamBuffer ioStreamBuf = IoStreamBuffer(this);

		private:
		// Custom defaulted default constructor behavior to add onto base class
		// constructor behavior.
		class _ConnectedSocketSpec {
			public:
			_ConnectedSocketSpec(ConnectedSocketSpec *that) {
				that->rdbuf(&that->ioStreamBuf);
			}
		};
		_ConnectedSocketSpec _connectedSocketSpec = _ConnectedSocketSpec(this);
	};
}

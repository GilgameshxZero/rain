// Extends the base Socket with the default specifications for TCP &
// SOCK_STREAM-based sockets.
#pragma once

#include "../socket.hpp"

#include <iostream>

namespace Rain::Networking::Tcp {
	// A base Socket with the assumption of TCP/stream. Comes with send/recv
	// buffers, and subclasses std::iostream.
	//
	// Since std::iostream does not utilize timeouts, the default timeouts from
	// Socket are used. Once they are surpassed, an error bit is set on the
	// underlying iostream. Subclasses may specify timeout behavior by overriding
	// send/recv.
	class Socket : public Networking::Socket, public std::iostream {
		private:
		// A custom subclass of std::streambuf as underlying the std::iostream.
		class IOStreamBuffer : public std::streambuf {
			private:
			// A reference to the incomplete "underlying" socket.
			Socket &socket;

			// Internal buffer to prevent delegating to kernel send/recv too often
			std::string recvBuffer, sendBuffer;

			public:
			IOStreamBuffer(
				Socket &socket,
				std::size_t recvBufferLen = 1_zu << 10,
				std::size_t sendBufferLen = 1_zu << 10)
					: socket(socket) {
				this->recvBuffer.reserve(recvBufferLen);
				// send buffer cannot be less than 1.
				this->sendBuffer.reserve(std::max(1_zu, sendBufferLen));
				this->sendBuffer.resize(this->sendBuffer.capacity());

				// Set internal pointers corresponding to empty buffers.
				this->setg(
					&this->recvBuffer[0], &this->recvBuffer[0], &this->recvBuffer[0]);

				// The final character in the sendBuffer is reserved for easy overflow
				// implementation.
				this->setp(
					&this->sendBuffer[0],
					&this->sendBuffer[0] + this->sendBuffer.size() - 1);
			}

			// Disable copy construct and assignment and any moves.
			IOStreamBuffer(IOStreamBuffer const &) = delete;
			IOStreamBuffer &operator=(IOStreamBuffer const &) = delete;

			// Allow move construct for moving Sockets.
			IOStreamBuffer(IOStreamBuffer &&other)
					: socket(other.socket),
						recvBuffer(std::move(other.recvBuffer)),
						sendBuffer(std::move(other.sendBuffer)) {}

			protected:
			// Ran out of characters while receiving from the socket.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					std::size_t recvStatus = SIZE_MAX;

					try {
						recvStatus = this->socket.recv(this->recvBuffer);
					} catch (...) {
						// recv throws (perhaps on peer abort) are consumed, setting the
						// buffer as invalid instead.
					}

					if (recvStatus == 0 || recvStatus == SIZE_MAX) {
						// The invariant this->gptr() == this->egptr() is maintained.
						return traits_type::eof();
					}

					this->setg(
						&this->recvBuffer[0],
						&this->recvBuffer[0],
						&this->recvBuffer[0] + this->recvBuffer.size());
				}

				return traits_type::to_int_type(*this->gptr());
			}

			// Write available buffer to the socket.
			virtual int sync() noexcept override {
				// Send available buffer.
				this->sendBuffer.resize(this->pptr() - this->pbase());
				try {
					this->socket.send(this->sendBuffer);
					this->sendBuffer.resize(this->sendBuffer.capacity());
				} catch (...) {
					// Failing on send (perhaps due to timeout) will set failbit on the
					// wrapper iostream.
					return -1;
				}

				// Mark the send buffer as empty.
				this->setp(
					&this->sendBuffer[0],
					&this->sendBuffer[0] + this->sendBuffer.size() - 1);

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
		IOStreamBuffer ioStreamBuf;

		public:
		Socket(
			// specification is provided as a parameter to adhere to parameter
			// contracts from specialization interfaces, but it is otherwise unused.
			Specification::Specification,
			bool interruptable = true,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10)
				: Networking::Socket(
						{pf,
						 Specification::SocketType::STREAM,
						 Specification::SocketProtocol::TCP},
						interruptable),
					std::iostream(&this->ioStreamBuf),
					ioStreamBuf(*this, recvBufferLen, sendBufferLen) {}

		// Base Socket contract: copy disabled, move constructor allowed.
		Socket(Socket const &) = delete;
		Socket &operator=(Socket const &) = delete;
		Socket(Socket &&other)
				: Networking::Socket(std::move(other)),
					std::iostream(&this->ioStreamBuf),
					ioStreamBuf(std::move(other.ioStreamBuf)) {}

		// Special constructor used by Worker on an accepted base Socket.
		Socket(
			Networking::Socket &&socket,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10)
				: Networking::Socket(std::move(socket)),
					std::iostream(&this->ioStreamBuf),
					ioStreamBuf(*this, recvBufferLen, sendBufferLen) {}

		// Virtualizes these signatures. Used by the iostream and other overrides
		// for send/recv. Subclasses should override these to provide for custom
		// behavior (timeouts, etc.).
		virtual std::size_t sendOnce(char const *msg, std::size_t msgLen) {
			return Networking::Socket::sendOnce(msg, msgLen);
		}
		virtual std::size_t send(char const *msg, std::size_t msgLen) {
			// Defaults to written in terms of this->sendOnce.
			std::size_t bytesSent = 0, sendOnceStatus;
			do {
				sendOnceStatus = this->sendOnce(msg + bytesSent, msgLen - bytesSent);
				bytesSent += sendOnceStatus;
			} while (bytesSent < msgLen && sendOnceStatus != 0);
			return bytesSent;
		}
		virtual std::size_t recv(char *buffer, std::size_t bufferLen) {
			return Networking::Socket::recv(buffer, bufferLen);
		}

		// Overrides base Socket send/recv to use virtualized functions.
		std::size_t sendOnce(std::string const &msg) {
			return this->sendOnce(msg.c_str(), msg.length());
		}
		std::size_t send(std::string const &msg) {
			return this->send(msg.c_str(), msg.length());
		}
		std::size_t recv(std::string &buffer) {
			buffer.resize(buffer.capacity());
			std::size_t recvStatus = this->recv(&buffer[0], buffer.length());
			if (recvStatus != SIZE_MAX) {
				buffer.resize(recvStatus);
			}
			return recvStatus;
		}
	};
}

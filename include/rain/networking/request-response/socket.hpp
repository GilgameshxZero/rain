// The R/R Socket protocol implements Workers/Clients where Clients send
// Requests and Workers send Responses.
//
// The R/R Socket protocol is abstract and cannot be instantiated directly.
// Thus, with the exception of Socket, only interface implementations are
// provided.
#pragma once

#include "../tcp/socket.hpp"

namespace Rain::Networking::RequestResponse {
	// The R/R Socket protocol implements Workers/Clients where Clients send
	// Requests and Workers send Responses.
	//
	// All connections begin with a Client Request.
	template <
		typename ProtocolRequest,
		typename ProtocolResponse,
		typename ProtocolClock = std::chrono::steady_clock>
	class Socket : public Tcp::Socket {
		public:
		typedef ProtocolRequest Request;
		typedef ProtocolResponse Response;
		typedef ProtocolClock Clock;
		typedef typename Clock::duration Duration;
		typedef typename Request::Message Message;

		private:
		typedef Socket<Request, Response, Clock> ThisSocket;

		// Request idle timeout. There can be at most this duration between the
		// finishing sending of a R/R and the finishing receiving of a R/R.
		Duration const maxRecvIdleDuration,
			// Timeout for sendOnce during sending a R/R.
			sendOnceTimeoutDuration;

		// Along with maxRecvIdleDuration, tracks the timeout of the next recv
		// operation on a R/R.
		Time::Timeout<Clock> recvTimeout;

		public:
		// Timeout exceptions.
		enum class Error : int { TIMEOUT_RECV = 1, TIMEOUT_SEND };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::RequestResponse::Socket";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::TIMEOUT_RECV:
						return "Socked timed out while idling for recv.";
					case Error::TIMEOUT_SEND:
						return "Socket timed out while idling for send.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		public:
		// Implements similar constructor arguments as TCP protocol sockets. Again,
		// specification is ignored.
		//
		// Two additional constructor arguments specify the timeout durations w.r.t.
		// R/R.
		Socket(
			Specification::Specification,
			bool interruptable = true,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s)
				: Tcp::Socket(
						Specification::Specification(),
						interruptable,
						pf,
						recvBufferLen,
						sendBufferLen),
					maxRecvIdleDuration(maxRecvIdleDuration),
					sendOnceTimeoutDuration(sendOnceTimeoutDuration),
					recvTimeout(maxRecvIdleDuration) {}
		Socket(Socket const &) = delete;
		Socket &operator=(Socket const &) = delete;
		Socket(Socket &&other)
				: Tcp::Socket(std::move(other)),
					maxRecvIdleDuration(other.maxRecvIdleDuration),
					sendOnceTimeoutDuration(other.sendOnceTimeoutDuration),
					recvTimeout(other.recvTimeout) {}

		// An additional constructor is called by Worker with a base Socket from
		// accept.
		Socket(
			Networking::Socket &&socket,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s)
				: Tcp::Socket(std::move(socket), recvBufferLen, sendBufferLen),
					maxRecvIdleDuration(maxRecvIdleDuration),
					sendOnceTimeoutDuration(sendOnceTimeoutDuration),
					recvTimeout(maxRecvIdleDuration) {}

		protected:
		// Hide TCP send/recv only for subclasses (interface limiters need access).
		// These call the later virtual internal send/recvs.
		using Tcp::Socket::sendOnce;
		using Tcp::Socket::send;
		using Tcp::Socket::recv;

		// Override TCP send/recv with proper timeout.
		//
		// Always succeeds on return, or throws on timeout/failure. Timeout in an
		// R/R subclass should always abort the Socket. recvTimeout must be updated
		// externally on recv of a full R/R.
		//
		// M/R/R should be friended so that they can access these protected
		// functions.
		virtual std::size_t sendOnce(char const *msg, std::size_t msgLen) override {
			if (msgLen == 0) {
				return 0;
			}

			std::size_t bytesSent = Networking::Socket::sendOnce(
				msg,
				msgLen,
				Time::Timeout<Clock>(this->sendOnceTimeoutDuration),
				SendFlag::NONE);

			if (bytesSent == 0) {
				// Timed out waiting for poll WRITE_NORMAL.
				throw Exception(Error::TIMEOUT_SEND);
			}
			return bytesSent;
		}
		virtual std::size_t recv(char *buffer, std::size_t bufferLen) override {
			std::size_t recvStatus = Networking::Socket::recv(
				buffer, bufferLen, this->recvTimeout, RecvFlag::NONE);
			if (recvStatus == SIZE_MAX) {
				throw Exception(Error::TIMEOUT_RECV);
			}
			return recvStatus;
		}

		public:
		// All send/recv on R/Rs redirect here, which go to sendWith/recvWith.
		//
		// Enable subclass postprocessing by letting them overload stream operators,
		// through which all send/recv happens.
		virtual ThisSocket &operator<<(Request &req) {
			req.sendWith(*this);
			return *this;
		}
		virtual ThisSocket &operator<<(Response &res) {
			res.sendWith(*this);
			return *this;
		}
		ThisSocket &operator<<(Request &&req) { return *this << req; }
		ThisSocket &operator<<(Response &&res) { return *this << res; }
		virtual ThisSocket &operator>>(Request &req) {
			req.recvWith(*this);
			this->recvTimeout = {this->maxRecvIdleDuration};
			return *this;
		}
		virtual ThisSocket &operator>>(Response &res) {
			res.recvWith(*this);
			this->recvTimeout = {this->maxRecvIdleDuration};
			return *this;
		}
		ThisSocket &operator>>(Request &&req) { return *this >> req; }
		ThisSocket &operator>>(Response &&res) { return *this >> res; }

		// Publicly, R/R Sockets can send/recv R/Rs, which also update the internal
		// recvTimeout accordingly. R/R Sockets can also send/recv R/Rs. These
		// utilities also track timeouts for the R/R Socket. Like the
		// sendWith/recvWith functions, always returns with success or throws on
		// timeout/failure.
		//
		// Sockets are upcasted to iostreams for R/R send/recv, avoiding circular
		// dependency b/t M/R/R and Socket and allowing parsing code to reside with
		// M/R/R.
		//
		// send/recv are aliased to no more than the stream operators themselves.
		void send(Request &req) { *this << req; }
		void send(Response &res) { *this << res; }

		// rvalue overloads for inline R/R construction.
		void send(Request &&req) { this->send(req); }
		void send(Response &&res) { this->send(res); }

		// NRVO/move guaranteed at least.
		Request recvRequest() {
			Request req;
			*this >> req;
			return req;
		}
		Response recvResponse() {
			Response res;
			*this >> res;
			return res;
		}

		void recv(Request &req) { *this >> req; }
		void recv(Response &res) { *this >> res; }
	};
}

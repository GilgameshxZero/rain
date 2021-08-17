// Worker specialization for R/R protocol Sockets.
#pragma once

#include "../tcp/worker.hpp"
#include "socket.hpp"

namespace Rain::Networking::ReqRes {
	class WorkerSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public Tcp::WorkerSocketSpecInterface {};
	// Worker specialization for TCP protocol Sockets.
	//
	// Enables pre/post-processing via overrides on stream operators.
	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class WorkerSocketSpecInterface
			: virtual public WorkerSocketSpecInterfaceInterface {
		public:
		using WorkerSocketSpecInterfaceInterface::send;
		using WorkerSocketSpecInterfaceInterface::recv;
		using WorkerSocketSpecInterfaceInterface::operator<<;
		using WorkerSocketSpecInterfaceInterface::operator>>;

		// Always completes with success, or throws or sets the stream into a bad
		// state on failure.
		//
		// Override here for pp-chaining, but limited to send/recv on Sockets.
		virtual void send(ResponseMessageSpec &res) { res.sendWith(*this); }
		virtual RequestMessageSpec &recv(RequestMessageSpec &req) {
			req.recvWith(*this);
			return req;
		}

		// For inline construction.
		void send(ResponseMessageSpec &&res) { this->send(res); }
		RequestMessageSpec recv() {
			RequestMessageSpec req;
			this->recv(req);
			return req;
		}

		// std::iostream overloads.
		ConnectedSocketSpecInterface &operator<<(ResponseMessageSpec &res) {
			this->send(res);
			return *this;
		}
		ConnectedSocketSpecInterface &operator>>(RequestMessageSpec &req) {
			this->recv(req);
			return *this;
		}

		// For inline construction.
		ConnectedSocketSpecInterface &operator<<(ResponseMessageSpec &&res) {
			return *this << res;
		}

		private:
		virtual void onWork() final override {
			// R/R protocol involves receiving a request, then processing it for a
			// response to send. Subclasses override the processing step.
			//
			// onRequest must not throw and respond internally. recvRequest may throw.
			//
			// Allow possibly sending a response as the first action upon connect.
			try {
				if (this->onInitialResponse()) {
					return;
				}
			} catch (...) {
				return;
			}

			while (this->good()) {
				// onRequest returns false to keep the connection open, and true to
				// abort it. Graceful closes should be handled in onRequest.
				try {
					RequestMessageSpec req;
					this->recv(req);
					if (this->onRequest(req)) {
						break;
					}
				} catch (...) {
					Rain::Error::consumeThrowable(
						[this]() { this->onRequestException(); }, RAIN_ERROR_LOCATION)();
					break;
				}
			}
		}

		// Inheriting classes should implement onCycle, which is responsible for
		// sending back a response if necessary. Return false to listen to next
		// request, true to attempt to gracefully close the connection.
		// Any throws will abort the connection.
		//
		// Takes rvalue reference here to simplify syntax in onWork; not const
		// reference because reading a R/R may modify it for the next reader (some
		// may only be read/transmitted once).
		virtual bool onRequest(RequestMessageSpec &req) = 0;

		// Optional exception handler does nothing by default. Should be noexcept.
		// throw and catch again inside this function to determine the exception
		// type. Guaranteed that this function is only called from within a catch
		// block, so throw; is safe.
		virtual void onRequestException() {}

		// Possibly send a response before the first request, after connecting.
		// Return true to abort.
		virtual bool onInitialResponse() { return false; }
	};

	// Worker specialization for TCP protocol Sockets.
	template <
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename Socket>
	class WorkerSocketSpec : public Socket,
													 virtual public WorkerSocketSpecInterface<
														 RequestMessageSpec,
														 ResponseMessageSpec>,
													 virtual public WorkerSocketSpecInterfaceInterface {
		using Socket::Socket;
	};

	// Shorthand for TCP Worker.
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
	class Worker : public WorkerSocketSpec<
									 RequestMessageSpec,
									 ResponseMessageSpec,
									 ConnectedSocketSpec<NamedSocketSpec<SocketSpec<Tcp::Worker<
										 sendBufferLen,
										 recvBufferLen,
										 sendTimeoutMs,
										 recvTimeoutMs,
										 SocketFamilyInterface,
										 SocketTypeInterface,
										 SocketProtocolInterface,
										 SocketOptions...>>>>> {
		using WorkerSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<NamedSocketSpec<SocketSpec<Tcp::Worker<
				sendBufferLen,
				recvBufferLen,
				sendTimeoutMs,
				recvTimeoutMs,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>>::WorkerSocketSpec;
	};
}

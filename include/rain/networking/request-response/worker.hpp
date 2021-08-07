// Worker specialization for R/R protocol Sockets.
#pragma once

#include "../tcp/worker.hpp"

namespace Rain::Networking::RequestResponse {
	// Worker specialization for TCP protocol Sockets.
	//
	// Enables pre/post-processing via overrides on stream operators.
	template <typename ProtocolSocket>
	class WorkerInterface : public Tcp::WorkerInterface<ProtocolSocket> {
		public:
		typedef ProtocolSocket Socket;

		// Alias Socket templates.
		using typename Socket::Request;
		using typename Socket::Response;
		using typename Socket::Clock;
		using typename Socket::Duration;
		using typename Socket::Message;

		private:
		// SuperInterface aliases the superclass.
		typedef Tcp::WorkerInterface<Socket> SuperInterface;

		public:
		// Interface aliases this class.
		typedef WorkerInterface<Socket> Interface;

		// Workers can only be constructed by their corresponding Server
		// workerFactory.
		//
		// Must provide default arguments for the additional parameters beyond the
		// first two, for compatibility with (unused) base Server workerFactory.
		template <typename... SocketArgs>
		WorkerInterface(
			Resolve::AddressInfo const &addressInfo,
			Networking::Socket &&socket,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s,
			SocketArgs &&...args)
				: SuperInterface(
						addressInfo,
						std::move(socket),
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration,
						std::forward<SocketArgs>(args)...) {}

		WorkerInterface(WorkerInterface const &) = delete;
		WorkerInterface &operator=(WorkerInterface const &) = delete;

		private:
		virtual void onWork() final override {
			// R/R protocol involves receiving a request, then processing it for a
			// response to send. Subclasses override the processing step.
			//
			// onRequest must not throw and respond internally. recvRequest may throw.
			Rain::Error::consumeThrowable(
				[this]() {
					// Allow possibly sending a response as the first action upon connect.
					if (this->onInitialResponse()) {
						this->abort();
						return;
					}

					while (this->good() && this->isValid()) {
						// onRequest returns false to keep the connection open, and true to
						// abort it. Graceful closes should be handled in onRequest.
						Request req;
						try {
							*this >> req;
						} catch (...) {
							this->onRecvRequestException();
							this->abort();
						}
						if (!this->good() || !this->isValid() || this->onRequest(req)) {
							break;
						}
					}

					// A possibly bad stream cannot be gracefully closed.
					this->abort();
				},
				RAIN_ERROR_LOCATION)();
		}

		// Inheriting classes should implement onRequest, which is responsible for
		// sending back a response if necessary. Return false to listen to next
		// request, true to attempt to gracefully close the connection.
		// Any throws will abort the connection.
		//
		// Takes rvalue reference here to simplify syntax in onWork; not const
		// reference because reading a R/R may modify it for the next reader (some
		// may only be read/transmitted once).
		virtual bool onRequest(Request &req) = 0;

		// Optional exception handler does nothing by default. Should be noexcept.
		// throw and catch again inside this function to determine the exception
		// type. Guaranteed that this function is only called from within a catch
		// block, so throw; is safe.
		virtual void onRecvRequestException() {}

		// Possibly send a response before the first request, after connecting.
		// Return true to abort.
		virtual bool onInitialResponse() { return false; }

		protected:
		// Chain tag.
		template <typename>
		struct Tag {};

		// TODO: Better access restrictions.
		protected:
		// Only allow a subset of streams.
		using SuperInterface::send;
		using SuperInterface::recv;
		using SuperInterface::recvResponse;

		// Subclass chains.
		virtual void streamOutImpl(Tag<Interface>, Response &) {}
		virtual void streamInImpl(Tag<Interface>, Request &) {}

		public:
		using SuperInterface::operator<<;
		using SuperInterface::operator>>;

		// Establish pre/post-processing chains via Tags on stream operators.
		virtual Interface &operator<<(Response &res) final override {
			this->streamOutImpl(Tag<Interface>(), res);
			SuperInterface::operator<<(res);
			return *this;
		}
		Interface &operator<<(Response &&res) {
			*this << res;
			return *this;
		}
		virtual Interface &operator>>(Request &req) final override {
			SuperInterface::operator>>(req);
			this->streamInImpl(Tag<Interface>(), req);
			return *this;
		}
		Interface &operator>>(Request &&req) {
			*this >> req;
			return *this;
		}

		// Reveal some overloads again.
		void send(Response &res) { SuperInterface::send(res); }
		void send(Response &&res) { SuperInterface::send(res); }
		void recv(Request &req) { SuperInterface::recv(req); }
	};
}

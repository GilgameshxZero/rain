// Client specialization for R/R sockets.
#pragma once

#include "../tcp/client.hpp"

namespace Rain::Networking::RequestResponse {
	// Client specialization for R/R sockets.
	//
	// Enables pre/post-processing via overrides on stream operators.
	template <typename ProtocolSocket>
	class ClientInterface : public Tcp::ClientInterface<ProtocolSocket> {
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
		typedef Tcp::ClientInterface<Socket> SuperInterface;

		public:
		// Interface aliases this class.
		typedef ClientInterface<Socket> Interface;

		// Maintains the contract that the Socket is created valid.
		template <typename... SocketArgs>
		ClientInterface(
			bool interruptable = true,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s,
			SocketArgs &&...args)
				: SuperInterface(
						interruptable,
						pf,
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration,
						std::forward<SocketArgs>(args)...) {}

		// Disable copy, allow move construct.
		ClientInterface(ClientInterface const &) = delete;
		ClientInterface &operator=(ClientInterface const &) = delete;
		ClientInterface(ClientInterface &&other)
				: SuperInterface(std::move(other)) {}

		// Allow direct move via base Socket.
		template <typename... SocketArgs>
		ClientInterface(Socket &&socket, SocketArgs &&...args)
				: SuperInterface(std::move(socket), std::forward<SocketArgs>(args)...) {
		}

		protected:
		// Chain tag.
		template <typename>
		struct Tag {};

		// TODO: Better access restrictions.
		protected:
		// Only allow a subset of streams.
		using SuperInterface::operator<<;
		using SuperInterface::operator>>;
		using SuperInterface::send;
		using SuperInterface::recv;
		using SuperInterface::recvRequest;

		// Subclass chains.
		virtual void streamOutImpl(Tag<Interface>, Request &) {}
		virtual void streamInImpl(Tag<Interface>, Response &) {}

		public:
		// Establish pre/post-processing chains via Tags on stream operators.
		virtual Interface &operator<<(Request &req) final override {
			this->streamOutImpl(Tag<Interface>(), req);
			SuperInterface::operator<<(req);
			return *this;
		}
		Interface &operator<<(Request &&req) {
			*this << req;
			return *this;
		}
		virtual Interface &operator>>(Response &res) final override {
			SuperInterface::operator>>(res);
			this->streamInImpl(Tag<Interface>(), res);
			return *this;
		}
		Interface &operator>>(Response &&res) {
			*this >> res;
			return *this;
		}

		// Reveal some overloads again.
		void send(Request &req) { SuperInterface::send(req); }
		void send(Request &&req) { SuperInterface::send(req); }
		void recv(Response &res) { SuperInterface::recv(res); }
	};

	// Final specializations may have dependent base class, so must qualify
	// dependent names again (unlike in TCP Client).
	//
	// R/R final specializations have little meaning since M/R/R is pure virtual.
}

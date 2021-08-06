// SMTP Client specialization.
#pragma once

#include "../request-response/client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// SMTP Client specialization.
	template <typename ProtocolSocket>
	class ClientInterface
			: public RequestResponse::ClientInterface<ProtocolSocket> {
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
		typedef RequestResponse::ClientInterface<Socket> SuperInterface;

		public:
		template <typename TagType>
		using Tag = typename SuperInterface::template Tag<TagType>;

		// Interface alias>es this class.
		typedef ClientInterface<Socket> Interface;

		// Same constructor.
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

		// Continue pp chains.
		private:
		virtual void streamOutImpl(Tag<Interface>, Request &) {}
		virtual void streamInImpl(Tag<Interface>, Response &) {}

		virtual void streamOutImpl(Tag<SuperInterface>, Request &req)
			final override {
			this->streamOutImpl(Tag<Interface>(), req);
			// No postprocessors.
		}
		virtual void streamInImpl(Tag<SuperInterface>, Response &res)
			final override {
			// No preprocessors.
			this->streamInImpl(Tag<Interface>(), res);
		}

		public:
		using SuperInterface::operator<<;
		using SuperInterface::operator>>;
	};

	typedef ClientInterface<Socket> Client;
}

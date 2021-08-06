// Client specialization for TCP sockets.
#pragma once

#include "../client.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	// Client specialization for TCP sockets.
	template <typename ProtocolSocket>
	class ClientInterface : public Networking::ClientInterface<ProtocolSocket> {
		private:
		// SuperInterface aliases the superclass.
		typedef Networking::ClientInterface<ProtocolSocket> SuperInterface;

		public:
		using typename SuperInterface::Socket;

		// Interface aliases this class.
		typedef ClientInterface<Socket> Interface;

		// Maintains the contract that the Socket is created valid.
		template <typename... SocketArgs>
		ClientInterface(
			bool interruptable = true,
			Specification::ProtocolFamily pf = Specification::ProtocolFamily::INET6,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			SocketArgs &&...args)
				: SuperInterface(
						Specification::Specification(),
						interruptable,
						pf,
						recvBufferLen,
						sendBufferLen,
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
	};

	// Final specialization for TCP Socket (without additional protocol layers).
	typedef ClientInterface<Socket> Client;
}

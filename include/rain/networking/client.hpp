// Socket specialization: the templated Client interface, and its protocol
// implementation with the basic Socket.
#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	// Socket specialization: the templated Client interface, and its protocol
	// implementation with the basic Socket.
	//
	// Clients satisfy the same NTA contract as does the Socket.
	//
	// Well-formed Clients NEVER block indefinitely. Upon any underlying Socket
	// operation timeout, the Client should abort without blocking.
	// This guarantees the NBTA contract on Clients.
	template <typename ProtocolSocket>
	class ClientInterface : public ProtocolSocket {
		public:
		typedef ProtocolSocket Socket;
		typedef ClientInterface<Socket> Interface;

		// *Suggested* not to use these functions, but they are always available in
		// the base class.
		private:
		using Socket::bind;
		using Socket::listen;
		using Socket::accept;

		public:
		// Constructor parameter list is based off of intended Socket, with
		// additional arguments for deriving classes and deriving ProtocolSockets.
		template <typename... SocketArgs>
		ClientInterface(
			Specification::Specification const &specification =
				{Specification::ProtocolFamily::INET6,
				 Specification::SocketType::STREAM,
				 Specification::SocketProtocol::TCP},
			bool interruptable = true,
			SocketArgs &&...args)
				: Socket(
						specification,
						interruptable,
						std::forward<SocketArgs>(args)...) {}

		// Similar copy/move specification to Socket.
		ClientInterface(ClientInterface const &) = delete;
		ClientInterface &operator=(ClientInterface const &) = delete;
		ClientInterface(ClientInterface &&other) : Socket(std::move(other)) {}

		// Allow direct move via base Socket.
		template <typename... SocketArgs>
		ClientInterface(Socket &&socket, SocketArgs &&...args)
				: Socket(std::move(socket), std::forward<SocketArgs>(args)...) {}
	};

	typedef ClientInterface<Socket> Client;
}

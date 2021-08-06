// Socket specialization: the templated Worker interface, and its protocol
// implementation with the basic Socket.
#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	// Socket specialization: the templated Worker interface, and its protocol
	// implementation with the basic Socket. It is spawned by Server accept.
	//
	// Workers satisfy the same NTA contract as Sockets.
	//
	// Well-formed Workers NEVER block indefinitely. Upon any underlying Socket
	// operation timeout, the Worker should abort without blocking.
	// This guarantees the NBTA contract on Servers which use the Worker.
	template <typename ProtocolSocket>
	class WorkerInterface : public ProtocolSocket {
		template <
			// Implementations of WorkerInterface should friend their corresponding
			// Server implementation so that onWork can be accessed.
			typename ServerInterfaceSocket,
			typename ServerInterfaceWorker>
		friend class ServerInterface;

		public:
		typedef ProtocolSocket Socket;
		typedef WorkerInterface<Socket> Interface;

		private:
		// Workers inherit the interrupt pair of the server by default, and this
		// cannot be overwritten. They cannot be interrupted except at the command
		// of the server.
		using Socket::setInterruptPair;
		using Socket::setNewInterruptPair;

		using Socket::connect;
		using Socket::bind;
		using Socket::listen;
		using Socket::accept;

		using Socket::interrupt;

		protected:
		// AddressInfo of the peer.
		Resolve::AddressInfo const addressInfo;

		public:
		// Construct a Worker from an accepted base Socket. Subclasses should follow
		// whatever signature the Server workerFactory uses.
		template <typename... SocketArgs>
		WorkerInterface(
			Resolve::AddressInfo const &addressInfo,
			Networking::Socket &&socket,
			SocketArgs &&...args)
				: Socket(std::move(socket), std::forward<SocketArgs>(args)...),
					addressInfo(addressInfo) {}

		// Workers are managed by the server, and should not be moved nor copied.
		// Move semantics are not implicitly made by the compiler once copy sematics
		// are deleted.
		WorkerInterface(WorkerInterface const &) = delete;
		WorkerInterface &operator=(WorkerInterface const &) = delete;

		// Private virtual: onWork can not be invoked directly, but can still be
		// overridden.
		private:
		virtual void onWork(){};
	};

	typedef WorkerInterface<Socket> Worker;
}

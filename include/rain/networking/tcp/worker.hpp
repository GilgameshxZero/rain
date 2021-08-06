// Worker specialization for TCP protocol Sockets.
#pragma once

#include "../worker.hpp"
#include "socket.hpp"

namespace Rain::Networking::Tcp {
	// Worker specialization for TCP protocol Sockets.
	template <typename ProtocolSocket>
	class WorkerInterface : public Networking::WorkerInterface<ProtocolSocket> {
		private:
		// SuperInterface aliases the superclass.
		typedef Networking::WorkerInterface<ProtocolSocket> SuperInterface;

		public:
		using typename SuperInterface::Socket;

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
			SocketArgs &&...args)
				: SuperInterface(
						addressInfo,
						std::move(socket),
						recvBufferLen,
						sendBufferLen,
						std::forward<SocketArgs>(args)...) {}

		// Workers cannot be copied nor moved.
		WorkerInterface(WorkerInterface const &) = delete;
		WorkerInterface &operator=(WorkerInterface const &) = delete;
	};

	typedef WorkerInterface<Socket> Worker;
}

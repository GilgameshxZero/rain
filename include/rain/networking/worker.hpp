// WorkerSocket(Interface) subclasses ConnectedSocket(Interface) with additional
// construction requirements only satisfiable by ServerSocket(Interface).
#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	class WorkerSocketSpecInterface
			: virtual public ConnectedSocketSpecInterface {
		// For access to onWork.
		template <typename, typename>
		friend class ServerSocketSpec;

		private:
		// Provides access to interrupter socket.
		virtual SocketInterface *interrupter() = 0;

		// Override the base SocketInterface poll with a two-Socket poll to listen
		// to interrupts from Server.
		virtual PollFlag poll(PollFlag event, Time::Timeout timeout = 15s)
			override {
			return SocketInterface::poll(
				{this, this->interrupter()},
				{event, PollFlag::READ_NORMAL},
				timeout)[0];
		}

		virtual void onWork() {}
	};
	// Socket specialization: the templated Worker interface, and its protocol
	// implementation with the basic Socket. It is spawned by Server accept.
	//
	// Workers satisfy the same NTA contract as Sockets.
	//
	// Well-formed Workers NEVER block indefinitely. Upon any underlying Socket
	// operation timeout, the Worker should abort without blocking.
	// This guarantees the NBTA contract on Servers which use the Worker.
	template <typename Socket>
	class WorkerSocketSpec : public Socket,
													 virtual public WorkerSocketSpecInterface {
		private:
		// Interrupt socket from the server.
		SocketInterface *_interrupter;

		virtual SocketInterface *interrupter() override {
			return this->_interrupter;
		}

		public:
		// Construct a Worker from an accepted base Socket. Subclasses should follow
		// whatever signature the Server workerFactory uses.
		WorkerSocketSpec(NativeSocket nativeSocket, SocketInterface *interrupter)
				: Socket(nativeSocket), _interrupter(interrupter) {}
	};

	// Shorthand which includes ConnectedSocket and NamedSocket and base Socket
	// templates.
	template <
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Worker
			: public WorkerSocketSpec<ConnectedSocketSpec<NamedSocketSpec<Socket<
					SocketFamilyInterface,
					SocketTypeInterface,
					SocketProtocolInterface,
					SocketOptions...>>>> {
		using WorkerSocketSpec<ConnectedSocketSpec<NamedSocketSpec<Socket<
			SocketFamilyInterface,
			SocketTypeInterface,
			SocketProtocolInterface,
			SocketOptions...>>>>::WorkerSocketSpec;
	};
}

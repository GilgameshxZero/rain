#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	template <typename SlaveType, typename RequestType, typename ResponseType>
	class Server
			: virtual protected RequestResponse::Socket<RequestType, ResponseType>,
				public Networking::Server<SlaveType> {
		// Shorthand for overriding virtuals in subclasses.
		public:
		typedef SlaveType Slave;
		typedef RequestType Request;
		typedef ResponseType Response;

		// Constructor.
		Server(std::size_t maxThreads = 128,
			std::size_t BUF_SZ = 16384,
			std::chrono::milliseconds const &ACCEPT_TIMEOUT_MS =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_MS_PER_KB =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_TIMEOUT_MS_LOWER =
				std::chrono::milliseconds(5000))
				: Networking::Socket(),
					RequestResponse::Socket<RequestType, ResponseType>(BUF_SZ,
						RECV_TIMEOUT_MS,
						SEND_MS_PER_KB,
						SEND_TIMEOUT_MS_LOWER),
					Networking::Server<SlaveType>(maxThreads, ACCEPT_TIMEOUT_MS) {}
					
		using Networking::Server<SlaveType>::getFamily;
		using Networking::Server<SlaveType>::getNativeSocket;
		using Networking::Server<SlaveType>::getProtocol;
		using Networking::Server<SlaveType>::getType;
		using Networking::Server<SlaveType>::isValid;
		using Networking::Server<SlaveType>::getService;

		protected:
		// Subclass behavior. Return true to abort Slave.
		virtual bool onRequest(Slave &, Request &) noexcept { return false; };

		private:
		// Superclass behavior.
		Slave *onServerAccept(Networking::Socket &acceptedSocket) override {
			return new Slave(acceptedSocket,
				this->BUF_SZ,
				this->RECV_TIMEOUT_MS,
				this->SEND_MS_PER_KB,
				this->SEND_TIMEOUT_MS_LOWER);
		}
		void onBeginSlaveTask(Slave &slave) noexcept override {
			// Respond to requests forever until either the request or response tells
			// us to abort.
			while (true) {
				Request req;
				if (slave.recv(req) || this->onRequest(slave, req)) {
					break;
				}
			}
		};
	};
}

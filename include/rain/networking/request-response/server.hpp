#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	template <typename RequestType, typename ResponseType>
	class Slave
			: public Networking::Slave,
				virtual public RequestResponse::Socket<RequestType, ResponseType> {
		public:
		// Needs to encompass same signature as Networking::Slave constructor, for
		// use in onServerAccept.
		Slave(Networking::Socket &socket,
			const std::chrono::milliseconds &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(1000),
			std::size_t BUF_SZ = 16384)
				: Networking::Socket(std::move(socket)),
					RequestResponse::Socket<RequestType, ResponseType>(socket,
						RECV_TIMEOUT_MS,
						BUF_SZ),
					Networking::Slave(socket) {}

		// Ambiguity resolution via exposure.
		using Networking::Slave::send;
		using RequestResponse::Socket<RequestType, ResponseType>::send;
		using Networking::Slave::recv;
		using RequestResponse::Socket<RequestType, ResponseType>::recv;
	};

	template <typename SlaveType, typename RequestType, typename ResponseType>
	class Server
			: public Networking::Server<SlaveType>,
				virtual protected RequestResponse::Socket<RequestType, ResponseType> {
		// Shorthand for overriding virtuals in subclasses.
		public:
		typedef SlaveType Slave;
		typedef RequestType Request;
		typedef ResponseType Response;

		// Settings.
		private:
		const std::chrono::milliseconds RECV_TIMEOUT_MS;
		const std::size_t BUF_SZ;

		// Constructor.
		public:
		Server(std::size_t maxThreads = 1024,
			const std::chrono::milliseconds &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(1000),
			std::size_t BUF_SZ = 16384)
				: Networking::Socket(),
					RequestResponse::Socket<RequestType, ResponseType>(RECV_TIMEOUT_MS,
						BUF_SZ),
					Networking::Server<SlaveType>(maxThreads),
					RECV_TIMEOUT_MS(RECV_TIMEOUT_MS),
					BUF_SZ(BUF_SZ) {}

		// Subclass behavior. Return true to abort Slave.
		virtual bool onRequest(Slave &, Request &) noexcept { return false; };

		// Superclass behavior.
		protected:
		Slave *onServerAccept(Networking::Socket &acceptedSocket) override {
			return new Slave(acceptedSocket, this->RECV_TIMEOUT_MS, this->BUF_SZ);
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

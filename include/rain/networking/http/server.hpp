#pragma once

#include "../request-response/server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	// Both base classes virtually inherit Socket, so there will only be one copy.
	class Slave : public RequestResponse::Slave<Request, Response>,
								virtual public Socket {
		public:
		// Cannot inherit constructor from Slave base since need non-default virtual
		// Networking::Socket constructor.
		Slave(Networking::Socket &socket,
			const std::chrono::milliseconds &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(60000),
			std::size_t BUF_SZ = 16384)
				: Networking::Socket(std::move(socket)),
					RequestResponse::Socket<Request, Response>(socket,
						RECV_TIMEOUT_MS,
						BUF_SZ),
					Socket(socket, RECV_TIMEOUT_MS, BUF_SZ),
					RequestResponse::Slave<Request, Response>(socket,
						RECV_TIMEOUT_MS,
						BUF_SZ) {}

		// Ambiguity resolution via exposure.
		using RequestResponse::Slave<Request, Response>::send;
		using Socket::send;
		using RequestResponse::Slave<Request, Response>::recv;
		using Socket::recv;
	};

	template <typename SlaveType>
	class Server : public RequestResponse::Server<SlaveType, Request, Response>,
								 virtual protected Socket {
		public:
		using RequestResponse::Server<SlaveType, Request, Response>::Server;
	};
}

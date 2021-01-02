// SMTP server implementation.
#pragma once

#include "../request-response/slave.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// Both base classes virtually inherit Socket, so there will only be one copy.
	class Slave : virtual protected Socket,
								public RequestResponse::Slave<Request, Response> {
		public:
		// Cannot inherit constructor from Slave base since need non-default virtual
		// Networking::Socket constructor.
		Slave(Networking::Socket &socket,
			std::size_t BUF_SZ = 16384,
			const std::chrono::milliseconds &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(5000),
			const std::chrono::milliseconds &SEND_MS_PER_KB =
				std::chrono::milliseconds(5000),
			const std::chrono::milliseconds &SEND_TIMEOUT_MS_LOWER =
				std::chrono::milliseconds(5000))
				: Networking::Socket(std::move(socket)),
					RequestResponse::Socket<Request, Response>(socket,
						BUF_SZ,
						RECV_TIMEOUT_MS,
						SEND_MS_PER_KB,
						SEND_TIMEOUT_MS_LOWER),
					Socket(socket,
						BUF_SZ,
						RECV_TIMEOUT_MS,
						SEND_MS_PER_KB,
						SEND_TIMEOUT_MS_LOWER),
					RequestResponse::Slave<Request, Response>(socket,
						BUF_SZ,
						RECV_TIMEOUT_MS,
						SEND_MS_PER_KB,
						SEND_TIMEOUT_MS_LOWER) {}

		using RequestResponse::Slave<Request, Response>::close;
		using RequestResponse::Slave<Request, Response>::getFamily;
		using RequestResponse::Slave<Request, Response>::getNativeSocket;
		using RequestResponse::Slave<Request, Response>::getProtocol;
		using RequestResponse::Slave<Request, Response>::getService;
		using RequestResponse::Slave<Request, Response>::getType;
		using RequestResponse::Slave<Request, Response>::isValid;
		using RequestResponse::Slave<Request, Response>::recv;
		using RequestResponse::Slave<Request, Response>::send;
		using RequestResponse::Slave<Request, Response>::shutdown;
	};
}

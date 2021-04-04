#pragma once

#include "../slave.hpp"
#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	template <typename RequestType, typename ResponseType>
	class Slave
			: virtual protected RequestResponse::Socket<RequestType, ResponseType>,
				private Networking::Slave {
		public:
		// Needs to encompass same signature as Networking::Slave constructor, for
		// use in onServerAccept.
		Slave(Networking::Socket &socket,
			std::size_t BUF_SZ = 16384,
			std::chrono::milliseconds const &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_MS_PER_KB =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_TIMEOUT_MS_LOWER =
				std::chrono::milliseconds(5000))
				: Networking::Socket(std::move(socket)),
					RequestResponse::Socket<RequestType, ResponseType>(socket,
						BUF_SZ,
						RECV_TIMEOUT_MS,
						SEND_MS_PER_KB,
						SEND_TIMEOUT_MS_LOWER),
					Networking::Slave(socket) {}

		// Expose relevant functions.
		using RequestResponse::Socket<RequestType, ResponseType>::close;
		using RequestResponse::Socket<RequestType, ResponseType>::getFamily;
		using RequestResponse::Socket<RequestType, ResponseType>::getNativeSocket;
		using RequestResponse::Socket<RequestType, ResponseType>::getProtocol;
		using RequestResponse::Socket<RequestType, ResponseType>::getService;
		using RequestResponse::Socket<RequestType, ResponseType>::getType;
		using RequestResponse::Socket<RequestType, ResponseType>::isValid;
		using RequestResponse::Socket<RequestType, ResponseType>::recv;
		using RequestResponse::Socket<RequestType, ResponseType>::send;
		using RequestResponse::Socket<RequestType, ResponseType>::shutdown;
	};
}

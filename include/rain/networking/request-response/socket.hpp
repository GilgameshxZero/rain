#pragma once

#include "../socket.hpp"

namespace Rain::Networking::RequestResponse {
	// Class with additional functions to send/receive requests/responses.
	template <typename RequestType, typename ResponseType>
	class Socket : virtual public Networking::Socket {
		public:
		// Request/response sockets have a buffer for their request/response
		// parsing.
		const std::chrono::milliseconds RECV_TIMEOUT_MS;
		const std::size_t BUF_SZ;
		char *buf;

		// Need to define default constructor on this virtual base class to ensure
		// that subclasses can more easily declare default constructors via `using`.
		Socket(const std::chrono::milliseconds &RECV_TIMEOUT_MS =
						 std::chrono::milliseconds(1000),
			std::size_t BUF_SZ = 16384)
				: Networking::Socket(),
					RECV_TIMEOUT_MS(RECV_TIMEOUT_MS),
					BUF_SZ(BUF_SZ),
					buf(new char[BUF_SZ]) {}
		Socket(Networking::Socket &socket,
			const std::chrono::milliseconds &RECV_TIMEOUT_MS,
			std::size_t BUF_SZ)
				: Networking::Socket(std::move(socket)),
					RECV_TIMEOUT_MS(RECV_TIMEOUT_MS),
					BUF_SZ(BUF_SZ),
					buf(new char[BUF_SZ]) {}
		~Socket() { delete[] buf; }

		// Return true when the Socket needs to be aborted.
		bool send(RequestType &req) noexcept { return req.sendWith(*this); }
		bool send(ResponseType &res) noexcept { return res.sendWith(*this); }
		bool recv(RequestType &req) noexcept { return req.recvWith(*this); }
		bool recv(ResponseType &res) noexcept { return res.recvWith(*this); }

		// Ambiguity resolution via exposure.
		using Networking::Socket::send;
		using Networking::Socket::recv;
	};
}

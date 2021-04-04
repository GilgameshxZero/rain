#pragma once

#include "../socket.hpp"

namespace Rain::Networking::RequestResponse {
	// Class with additional functions to send/receive requests/responses.
	template <typename RequestType, typename ResponseType>
	class Socket : virtual protected Networking::Socket {
		public:
		// Request/response sockets have a buffer for their request/response
		// parsing.
		std::size_t const BUF_SZ;
		char *buf;
		std::chrono::milliseconds const
			RECV_TIMEOUT_MS,	// If no data is sent over this period, the socket will
												// likely be closed. If data is sent every period, then
												// the socket is guaranteed to be open.
			SEND_MS_PER_KB,	 // `send` timeout scales linearly with amount of data
											 // sent...
			SEND_TIMEOUT_MS_LOWER;	// ...down to a lower limit.

		// Need to define default constructor on this virtual base class to ensure
		// that subclasses can more easily declare default constructors via `using`.
		Socket(std::size_t BUF_SZ = 16384,
			std::chrono::milliseconds const &RECV_TIMEOUT_MS =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_MS_PER_KB =
				std::chrono::milliseconds(5000),
			std::chrono::milliseconds const &SEND_TIMEOUT_MS_LOWER =
				std::chrono::milliseconds(5000))
				: Networking::Socket(),
					BUF_SZ(BUF_SZ),
					buf(new char[BUF_SZ]),
					RECV_TIMEOUT_MS(RECV_TIMEOUT_MS),
					SEND_MS_PER_KB(SEND_MS_PER_KB),
					SEND_TIMEOUT_MS_LOWER(SEND_TIMEOUT_MS_LOWER) {}
		Socket(Networking::Socket &socket,
			std::size_t BUF_SZ,
			std::chrono::milliseconds const &RECV_TIMEOUT_MS,
			std::chrono::milliseconds const &SEND_MS_PER_KB,
			std::chrono::milliseconds const &SEND_TIMEOUT_MS_LOWER)
				: Networking::Socket(std::move(socket)),
					BUF_SZ(BUF_SZ),
					buf(new char[BUF_SZ]),
					RECV_TIMEOUT_MS(RECV_TIMEOUT_MS),
					SEND_MS_PER_KB(SEND_MS_PER_KB),
					SEND_TIMEOUT_MS_LOWER(SEND_TIMEOUT_MS_LOWER) {}
		~Socket() { delete[] buf; }

		// Return true when the Socket needs to be aborted.
		bool send(RequestType &req) noexcept { return req.sendWith(*this); }
		bool send(ResponseType &res) noexcept { return res.sendWith(*this); }
		bool recv(RequestType &req) noexcept { return req.recvWith(*this); }
		bool recv(ResponseType &res) noexcept { return res.recvWith(*this); }

		// Override base Socket send and recv to take into account the timeout
		// options. In addition, returns true on failure or timeout.
		bool send(char const *msg,
			std::size_t len = 0,
			Networking::Socket::SendFlag flags =
				Networking::Socket::SendFlag::NONE) const noexcept {
			if (len == 0) {
				len = std::strlen(msg);
			}

			try {
				return Networking::Socket::send(msg,
								 len,
								 flags,
								 std::max(
									 std::chrono::milliseconds(len / 1000 * this->SEND_MS_PER_KB),
									 this->SEND_TIMEOUT_MS_LOWER)) < len;
			} catch (...) {
				return true;
			}
		}
		bool send(std::string const &s,
			Networking::Socket::SendFlag flags =
				Networking::Socket::SendFlag::NONE) const noexcept {
			return this->send(s.c_str(), s.length(), flags);
		}
		// Returns 0 on error or timeout.
		std::size_t recv(char *buf,
			std::size_t len,
			Networking::Socket::RecvFlag flags =
				Networking::Socket::RecvFlag::NONE) const noexcept {
			try {
				// This may be a different buf than the one in the Socket.
				return Networking::Socket::recv(buf, len, flags, this->RECV_TIMEOUT_MS);
			} catch (...) {
				return 0;
			}
		}

		// Expose relevant functions.
		using Networking::Socket::accept;
		using Networking::Socket::bind;
		using Networking::Socket::close;
		using Networking::Socket::connect;
		using Networking::Socket::getFamily;
		using Networking::Socket::getNativeSocket;
		using Networking::Socket::getProtocol;
		using Networking::Socket::getService;
		using Networking::Socket::getType;
		using Networking::Socket::isValid;
		using Networking::Socket::listen;
		using Networking::Socket::shutdown;
	};
}

#pragma once

#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	// Any *With functions return true when the Socket needs to be aborted.

	template <typename RequestType, typename ResponseType>
	class Request {
		public:
		virtual bool sendWith(Socket<RequestType, ResponseType> &) noexcept = 0;
		virtual bool recvWith(Socket<RequestType, ResponseType> &) noexcept = 0;
	};

	template <typename RequestType, typename ResponseType>
	class Response {
		public:
		virtual bool sendWith(Socket<RequestType, ResponseType> &) noexcept = 0;
		virtual bool recvWith(Socket<RequestType, ResponseType> &) noexcept = 0;
	};
}

// Common utilities for SMTP server and client sockets.
#pragma once

#include "../request-response/socket.hpp"
#include "request.hpp"
#include "response.hpp"

namespace Rain::Networking::Smtp {
	class Socket : virtual public RequestResponse::Socket<Request, Response> {
		public:
		using RequestResponse::Socket<Request, Response>::Socket;
	};
}

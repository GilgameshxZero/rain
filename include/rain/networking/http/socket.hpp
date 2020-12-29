#pragma once

#include "../request-response/socket.hpp"
#include "request-response.hpp"

namespace Rain::Networking::Http {
	class Socket : virtual public RequestResponse::Socket<Request, Response> {
		public:
		using RequestResponse::Socket<Request, Response>::Socket;
	};
}

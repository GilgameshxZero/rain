#pragma once

#include "../request-response/client.hpp"
#include "request.hpp"
#include "response.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	// TODO.
	class Client : virtual protected Socket,
								 public RequestResponse::Client<Request, Response> {
		public:
		using RequestResponse::Client<Request, Response>::Client;

		using RequestResponse::Client<Request, Response>::close;
		using RequestResponse::Client<Request, Response>::connect;
		using RequestResponse::Client<Request, Response>::getFamily;
		using RequestResponse::Client<Request, Response>::getNativeSocket;
		using RequestResponse::Client<Request, Response>::getProtocol;
		using RequestResponse::Client<Request, Response>::getService;
		using RequestResponse::Client<Request, Response>::getType;
		using RequestResponse::Client<Request, Response>::isValid;
		using RequestResponse::Client<Request, Response>::recv;
		using RequestResponse::Client<Request, Response>::send;
		using RequestResponse::Client<Request, Response>::shutdown;
	};
}

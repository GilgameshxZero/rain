#pragma once

#include "../request-response/server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Http {
	template <typename SlaveType>
	class Server : virtual protected Socket,
								 public RequestResponse::Server<SlaveType, Request, Response> {
		public:
		using RequestResponse::Server<SlaveType, Request, Response>::Server;
		
		using RequestResponse::Server<SlaveType, Request, Response>::getFamily;
		using RequestResponse::Server<SlaveType, Request, Response>::getNativeSocket;
		using RequestResponse::Server<SlaveType, Request, Response>::getProtocol;
		using RequestResponse::Server<SlaveType, Request, Response>::getType;
		using RequestResponse::Server<SlaveType, Request, Response>::isValid;
		using RequestResponse::Server<SlaveType, Request, Response>::getService;
	};
}

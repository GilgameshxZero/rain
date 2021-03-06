#pragma once

#include "../client.hpp"
#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	template <typename RequestType, typename ResponseType>
	class Client
			: virtual protected RequestResponse::Socket<RequestType, ResponseType>,
				private Networking::Client {
		public:
		using RequestResponse::Socket<RequestType, ResponseType>::Socket;

		// Expose relevant functions.
		using RequestResponse::Socket<RequestType, ResponseType>::close;
		using RequestResponse::Socket<RequestType, ResponseType>::connect;
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

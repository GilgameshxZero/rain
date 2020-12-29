#pragma once

#include "../client.hpp"
#include "socket.hpp"

namespace Rain::Networking::RequestResponse {
	template <typename RequestType, typename ResponseType>
	class Client
			: public Networking::Client,
				virtual public RequestResponse::Socket<RequestType, ResponseType> {
		public:
		using RequestResponse::Socket<RequestType, ResponseType>::Socket;

		// Ambiguity resolution via exposure.
		using RequestResponse::Socket<RequestType, ResponseType>::send;
		using Networking::Client::send;
		using RequestResponse::Socket<RequestType, ResponseType>::recv;
		using Networking::Client::recv;
	};
}

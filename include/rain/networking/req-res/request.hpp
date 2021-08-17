// Abstract Request for R/R Sockets.
#pragma once

#include "message.hpp"

#include <iostream>

namespace Rain::Networking::ReqRes {
	class RequestMessageSpecInterface : virtual public MessageInterface {};

	// Abstract Request for R/R Sockets. See MessageInterface for details on
	// intended usage.
	template <typename Message>
	class RequestMessageSpec : public Message,
														 virtual public RequestMessageSpecInterface {
		using Message::Message;
	};

	class Request : public RequestMessageSpec<Message> {
		using RequestMessageSpec<Message>::RequestMessageSpec;
	};
}

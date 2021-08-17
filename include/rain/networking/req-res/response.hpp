// Abstract Response for R/R Sockets.
#pragma once

#include "message.hpp"

#include <iostream>

namespace Rain::Networking::ReqRes {
	class ResponseMessageSpecInterface : virtual public MessageInterface {};

	// Abstract Request for R/R Sockets. See MessageInterface for details on
	// intended usage.
	template <typename Message>
	class ResponseMessageSpec : public Message,
															virtual public ResponseMessageSpecInterface {
		using Message::Message;
	};

	class Response : public ResponseMessageSpec<Message> {
		using ResponseMessageSpec<Message>::ResponseMessageSpec;
	};
}

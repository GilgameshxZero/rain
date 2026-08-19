// Request-specific IMAP parsing.
#pragma once

#include "../../literal.hpp"
#include "../req_res/request.hpp"
#include "message.hpp"

namespace Rain::Networking::Imap {
	class RequestMessageSpecInterface :
		virtual public MessageSpecInterface,
		virtual public ReqRes::RequestMessageSpecInterface {};

	template<typename Message>
	class RequestMessageSpec :
		public Message,
		virtual public RequestMessageSpecInterface {
		public:
		using Message::Message;
	};

	// Shorthand.
	class Request :
		public RequestMessageSpec<
			MessageSpec<ReqRes::Request>> {
		using RequestMessageSpec<
			MessageSpec<ReqRes::Request>>::RequestMessageSpec;
	};
}

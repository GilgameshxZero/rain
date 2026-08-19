// Response-specific IMAP parsing.
#pragma once

#include "../../literal.hpp"
#include "../req_res/response.hpp"
#include "message.hpp"

namespace Rain::Networking::Imap {
	class ResponseMessageSpecInterface :
		virtual public MessageSpecInterface,
		virtual public ReqRes::ResponseMessageSpecInterface {};

	template<typename Message>
	class ResponseMessageSpec :
		public Message,
		virtual public ResponseMessageSpecInterface {
		public:
		using Message::Message;
	};

	// Shorthand.
	class Response :
		public ResponseMessageSpec<
			MessageSpec<ReqRes::Response>> {
		using ResponseMessageSpec<
			MessageSpec<ReqRes::Response>>::ResponseMessageSpec;
	};
}

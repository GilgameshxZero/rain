// Shared functionality between Smtp R/R.
#pragma once

#include "../../string.hpp"
#include "../req-res/message.hpp"

namespace Rain::Networking::Smtp {
	class MessageSpecInterface : virtual public ReqRes::MessageInterface {};

	template <typename Message>
	class MessageSpec : public Message, virtual public MessageSpecInterface {
		using Message::Message;
	};
}

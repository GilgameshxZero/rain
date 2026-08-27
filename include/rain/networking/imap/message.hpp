#pragma once

#include "../../string.hpp"
#include "../req_res/message.hpp"

namespace Rain::Networking::Imap {
	class MessageSpecInterface :
		virtual public ReqRes::MessageInterface {};

	template<typename Message>
	class MessageSpec :
		public Message,
		virtual public MessageSpecInterface {
		using Message::Message;

		public:
		// Assumes well-formed tag (no spaces and special
		// characters).
		std::string tag;

		MessageSpec(std::string const &tag) : tag{tag} {}
		MessageSpec(MessageSpec &&other) :
			tag{std::move(other.tag)} {}

		virtual void sendWith(std::ostream &stream) override {
			stream << tag << ' ';
		}
		virtual void recvWith(std::istream &stream) override {
			std::getline(stream, this->tag, ' ');
		}
	};
}

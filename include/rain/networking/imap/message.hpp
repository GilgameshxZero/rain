// Shared functionality between IMAP R/R.
#pragma once

#include "../../string.hpp"
#include "../req_res/message.hpp"
#include "command.hpp"

namespace Rain::Networking::Imap {
	class MessageSpecInterface :
		virtual public ReqRes::MessageInterface {
		public:
		std::string tag;
		Command command;
		std::string data;

		MessageSpecInterface(
			std::string const &tag = "*",
			Command command = Command::NONE,
			std::string const &data = "") :
			tag{tag},
			command{command},
			data{data} {}
		MessageSpecInterface(MessageSpecInterface &&other) :
			tag{std::move(other.tag)},
			command{std::move(other.command)},
			data{std::move(other.data)} {}

		virtual void sendWith(std::ostream &stream) override {
			// TODO: Literal data is treated differently.
			stream << this->tag << ' ' << this->command
						 << (this->command == Command::NONE ? "" : " ")
						 << this->data << "\r\n";
		}
		virtual void recvWith(std::istream &) override {
			// TODO.
		}
	};

	template<typename Message>
	class MessageSpec :
		public Message,
		virtual public MessageSpecInterface {
		using Message::Message;
	};
}

// Response-specific IMAP parsing.
#pragma once

#include "../../literal.hpp"
#include "../req_res/response.hpp"
#include "command.hpp"
#include "message.hpp"
#include "response_command.hpp"

// TODO: Implement polymorphic types (`ResponseCommand`).

namespace Rain::Networking::Imap {
	class ResponseMessageSpecInterface :
		virtual public MessageSpecInterface,
		virtual public ReqRes::ResponseMessageSpecInterface {};

	template<typename Message>
	class ResponseMessageSpec :
		public Message,
		virtual public ResponseMessageSpecInterface {
		public:
		std::unique_ptr<ResponseCommand> command;

		ResponseMessageSpec() : Message("*") {}
		ResponseMessageSpec(std::string const &tag) :
			Message(tag) {}

		virtual void sendWith(std::ostream &stream) override {
			Message::sendWith(stream);
			this->command->sendWith(stream);
		}
		virtual void recvWith(std::istream &stream) override {
			Message::recvWith(stream);
			// Create a command based on the command string.
			std::string commandString;
			stream >> commandString;
			static std::unordered_map<
				std::string,
				ResponseCommand *(*)()> const FACTORY{
				{"CAPABILITY", &factoryCreate<Command::Capability>},
				{"LIST", &factoryCreate<Command::List>},
				{"OK", &factoryCreate<Command::Ok>},
				{"BAD", &factoryCreate<Command::Bad>}};
			this->command.reset(FACTORY.at(commandString)());
			this->command->recvWith(stream);
		}

		private:
		// Helper to force return type.
		template<typename Type>
		static ResponseCommand *factoryCreate() {
			return new Type();
		}
	};

	// Shorthand.
	class Response :
		public ResponseMessageSpec<
			MessageSpec<ReqRes::Response>> {
		using ResponseMessageSpec<
			MessageSpec<ReqRes::Response>>::ResponseMessageSpec;
	};
}

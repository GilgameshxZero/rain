// Request-specific IMAP parsing.
#pragma once

#include "../../literal.hpp"
#include "../req_res/request.hpp"
#include "command.hpp"
#include "message.hpp"
#include "request_command.hpp"

// TODO: Implement polymorphic types (`RequestCommand`).

/*
<https://www.rfc-editor.org/info/rfc9051>

CAPABILITY
NOOP
LOGOUT

LOGIN

LIST
SELECT

(FETCH
SEARCH
STORE
COPY
EXPUNGE)
UID
APPEND
*/

namespace Rain::Networking::Imap {
	class RequestMessageSpecInterface :
		virtual public MessageSpecInterface,
		virtual public ReqRes::RequestMessageSpecInterface {};

	template<typename Message>
	class RequestMessageSpec :
		public Message,
		virtual public RequestMessageSpecInterface {
		public:
		std::unique_ptr<RequestCommand> command;

		// Default generate a random tag for this message.
		RequestMessageSpec() : Message("TODO") {}
		RequestMessageSpec(std::string const &tag) :
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
				RequestCommand *(*)()> const FACTORY{
				{"CAPABILITY", &factoryCreate<Command::Capability>},
				{"NOOP", &factoryCreate<Command::Noop>},
				{"LOGOUT", &factoryCreate<Command::Logout>},
				{"AUTHENTICATE",
					&factoryCreate<Command::Authenticate>},
				{"LOGIN", &factoryCreate<Command::Login>},
				{"LIST", &factoryCreate<Command::List>},
				{"SELECT", &factoryCreate<Command::Select>},
				{"FETCH", &factoryCreate<Command::Fetch>}};
			this->command.reset(FACTORY.at(commandString)());
			this->command->recvWith(stream);
		}

		private:
		// Helper to force return type.
		template<typename Type>
		static RequestCommand *factoryCreate() {
			return new Type();
		}
	};

	// Shorthand.
	class Request :
		public RequestMessageSpec<
			MessageSpec<ReqRes::Request>> {
		using RequestMessageSpec<
			MessageSpec<ReqRes::Request>>::RequestMessageSpec;
	};
}

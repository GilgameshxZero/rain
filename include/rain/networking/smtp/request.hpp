// Request-specific SMTP parsing.
#pragma once

#include "../req-res/request.hpp"
#include "command.hpp"
#include "message.hpp"

namespace Rain::Networking::Smtp {
	class RequestMessageSpecInterface
			: virtual public MessageSpecInterface,
				virtual public ReqRes::RequestMessageSpecInterface {
		public:
		// Exception class required for catching from R/R onWork.
		enum class Error { SYNTAX_ERROR_COMMAND = 1 };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Smtp::Request";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::SYNTAX_ERROR_COMMAND:
						return "Syntax error, command unrecognized.";
					default:
						return "Generic.";
				}
			}
		};
		using Exception = Rain::Error::Exception<Error, ErrorCategory>;
	};

	template <typename Message>
	class RequestMessageSpec : public Message,
														 virtual public RequestMessageSpecInterface {
		public:
		// Command/verb of Request.
		Command command;

		// Single-line parameter for command.
		std::string parameter;

		// Carry over constructor which recvs from a Socket.
		RequestMessageSpec(
			Command command = Command::HELO,
			std::string const &parameter = "")
				: Message(), command(command), parameter(parameter) {}
		RequestMessageSpec(RequestMessageSpec &&other)
				: Message(std::move(other)),
					command(other.command),
					parameter(std::move(other.parameter)) {}

		// Overrides for Super versions implement protocol behavior.
		//
		// Calls (almost) no-op Message sendWith/recvWith to check transmissability.
		virtual void sendWith(std::ostream &stream) override {
			stream << this->command;
			if (!this->parameter.empty()) {
				stream << " " << this->parameter;
			}
			stream << "\r\n";
			stream.flush();
		}
		virtual void recvWith(std::istream &stream) override {
			// Command is always first four characters.
			try {
				std::string commandStr(4, '\0');
				stream.read(&commandStr[0], commandStr.length());
				this->command = Command(commandStr);
			} catch (...) {
				throw Exception(Error::SYNTAX_ERROR_COMMAND);
			}

			// Next character is a space.
			stream.get();

			// After that, it is the parameter. No longer than 1K allowed.
			this->parameter.resize(1_zu << 10);
			stream.getline(&this->parameter[0], this->parameter.length());
			// -2 to discard the \r\0.
			this->parameter.resize(static_cast<std::size_t>(
				std::max(std::streamsize(0), stream.gcount() - 2)));
		}
	};

	// Shorthand.
	class Request : public RequestMessageSpec<MessageSpec<ReqRes::Request>> {
		using RequestMessageSpec<MessageSpec<ReqRes::Request>>::RequestMessageSpec;
	};
}

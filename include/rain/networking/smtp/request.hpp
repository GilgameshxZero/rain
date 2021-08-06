// Request-specific SMTP parsing.
#pragma once

#include "../request-response/request.hpp"
#include "command.hpp"
#include "message.hpp"

namespace Rain::Networking::Smtp {
	template <typename ProtocolMessage>
	class RequestInterface
			: public RequestResponse::RequestInterface<ProtocolMessage> {
		public:
		typedef ProtocolMessage Message;

		private:
		// SuperInterface aliases the superclass.
		typedef RequestResponse::RequestInterface<Message> SuperInterface;

		// Interface aliases this class.
		typedef RequestInterface<Message> Interface;

		protected:
		// Tag aliases for sendWith/recvWith convenience.
		typedef typename SuperInterface::InterfaceTag SuperTag;
		typedef typename SuperInterface::template Tag<Interface> InterfaceTag;

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
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		// Command/verb of Request.
		Command command;

		// Single-line parameter for command.
		std::string parameter;

		// Carry over constructor which recvs from a Socket.
		template <typename... MessageArgs>
		RequestInterface(
			Command command = Command::HELO,
			std::string const &parameter = "",
			MessageArgs &&...args)
				: SuperInterface(
						// bind version to rvalue reference to be perfect forwarded by
						// SuperInterface to Message.
						std::forward<MessageArgs>(args)...),
					command(command),
					parameter(parameter) {}
		RequestInterface(RequestInterface &&other)
				: SuperInterface(std::move(other)),
					command(other.command),
					parameter(std::move(other.parameter)) {}

		// Not copyable.
		RequestInterface(RequestInterface const &) = delete;
		RequestInterface &operator=(RequestInterface const &) = delete;

		private:
		// Provided for subclass override.
		virtual void sendWithImpl(InterfaceTag, std::ostream &){};
		virtual void recvWithImpl(InterfaceTag, std::istream &){};

		// Overrides for Super versions implement protocol behavior.
		//
		// Calls (almost) no-op Message sendWith/recvWith to check transmissability.
		virtual void sendWithImpl(SuperTag, std::ostream &stream) final override {
			this->sendWithImpl(InterfaceTag(), stream);

			stream << this->command;
			if (!this->parameter.empty()) {
				stream << " " << this->parameter;
			}
			stream << "\r\n";
			stream.flush();
		}
		virtual void recvWithImpl(SuperTag, std::istream &stream) final override {
			this->recvWithImpl(InterfaceTag(), stream);

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
			this->parameter.resize(std::max(std::streamsize(0), stream.gcount() - 2));
		}
	};

	// Request final specialization.
	typedef RequestInterface<Message> Request;
}

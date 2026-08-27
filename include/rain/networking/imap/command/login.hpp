#pragma once

#include "../request_command.hpp"

namespace Rain::Networking::Imap::Command {
	class Login : public RequestCommand {
		public:
		std::string buffer;

		Login(std::string const &buffer = "") :
			buffer{buffer} {}

		virtual std::string commandString() override final {
			return "LOGIN";
		}
		virtual void sendCommandWith(
			std::ostream &stream) override final {
			stream << buffer;
		}
		virtual void recvCommandWith(
			std::istream &stream) override final {
			std::getline(stream, buffer);
		}
	};
}

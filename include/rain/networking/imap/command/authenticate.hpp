#pragma once

#include "../request_command.hpp"

namespace Rain::Networking::Imap::Command {
	// Dummy command.
	class Authenticate : public RequestCommand {
		public:
		std::string buffer;

		Authenticate(std::string const &buffer = "") :
			buffer{buffer} {}

		virtual std::string commandString() override final {
			return "AUTHENTICATE";
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

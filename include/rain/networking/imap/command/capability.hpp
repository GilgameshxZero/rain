#pragma once

#include "../request_command.hpp"
#include "../response_command.hpp"

namespace Rain::Networking::Imap::Command {
	class Capability :
		public RequestCommand,
		public ResponseCommand {
		public:
		std::string buffer;

		Capability(std::string const &buffer = "") :
			buffer{buffer} {}

		virtual std::string commandString() override final {
			return "CAPABILITY";
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

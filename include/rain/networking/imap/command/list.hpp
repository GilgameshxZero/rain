#pragma once

#include "../request_command.hpp"
#include "../response_command.hpp"

namespace Rain::Networking::Imap::Command {
	class List :
		public RequestCommand,
		public ResponseCommand {
		public:
		std::string buffer;

		List(std::string const &buffer = "") : buffer{buffer} {}

		virtual std::string commandString() override final {
			return "LIST";
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

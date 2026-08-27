#pragma once

#include "../response_command.hpp"

namespace Rain::Networking::Imap::Command {
	class Ok : public ResponseCommand {
		public:
		std::string buffer;

		Ok(std::string const &buffer = "") : buffer{buffer} {}

		virtual std::string commandString() override final {
			return "OK";
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

#pragma once

#include "../request_command.hpp"

namespace Rain::Networking::Imap::Command {
	class Noop : public RequestCommand {
		public:
		std::string buffer;

		virtual std::string commandString() override final {
			return "NOOP";
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

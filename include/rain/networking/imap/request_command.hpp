#pragma once

#include "../../string.hpp"

#include <iostream>

namespace Rain::Networking::Imap {
	class RequestCommand {
		public:
		virtual ~RequestCommand() = default;

		void sendWith(std::ostream &stream) {
			stream << this->commandString() << ' ';
			this->sendCommandWith(stream);
			stream << "\r\n" << std::flush;
		}
		// `recv` everything beginning from after the space of
		// the command string.
		void recvWith(std::istream &stream) {
			this->recvCommandWith(stream);
		}

		virtual std::string commandString() = 0;
		virtual void sendCommandWith(std::ostream &) = 0;
		virtual void recvCommandWith(std::istream &) = 0;
	};
}

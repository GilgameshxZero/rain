#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	class Client : virtual protected Socket {
		public:
		Client(Family family = Family::IPV4,
			Type type = Type::STREAM,
			Protocol protocol = Protocol::TCP)
				: Socket(family, type, protocol) {}

		// Expose some functions from base Socket.
		using Socket::connect;
		using Socket::getNativeSocket;
		using Socket::getFamily;
		using Socket::getType;
		using Socket::getProtocol;
		using Socket::send;
		using Socket::recv;
		using Socket::shutdown;
		using Socket::close;
	};
}

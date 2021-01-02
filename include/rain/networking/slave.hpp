#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	// A new Slave is spawned from each accepted connection from the server.
	class Slave : virtual protected Socket {
		public:
		Slave(Socket &socket) : Socket(std::move(socket)) {}

		// Expose relevant functions.
		using Socket::close;
		using Socket::getFamily;
		using Socket::getNativeSocket;
		using Socket::getProtocol;
		using Socket::getService;
		using Socket::getType;
		using Socket::isValid;
		using Socket::recv;
		using Socket::send;
		using Socket::shutdown;
	};
}

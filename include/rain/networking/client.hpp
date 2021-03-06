#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	// A restricted Socket.
	class Client : virtual protected Socket {
		public:
		using Socket::Socket;

		// Expose relevant functions.
		using Socket::close;
		using Socket::connect;
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

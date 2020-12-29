#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	// A restricted Socket.
	class Client : virtual protected Socket {
		public:
		using Socket::Socket;
		using Socket::getNativeSocket;
		using Socket::getFamily;
		using Socket::getType;
		using Socket::getProtocol;
		using Socket::isValid;
		using Socket::connect;
		using Socket::getService;
		using Socket::send;
		using Socket::recv;
		using Socket::shutdown;
		using Socket::close;
	};
}

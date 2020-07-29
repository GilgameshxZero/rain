// SMTP server implementation.
#pragma once

#include "../server.hpp"
#include "socket.hpp"

namespace Rain::Networking::Smtp {
	// Forward declaration.
	class Server;

	class ServerSlave
			: public Smtp::Socket,
				public CustomServerSlave<Smtp::Server, Smtp::ServerSlave> {
		public:
		// Buffer used internally by Server.
		std::size_t bufSz = 0;
		char *buf = NULL;

		ServerSlave(const Networking::Socket &socket, Smtp::Server *server)
				: Smtp::Socket(socket), CustomServerSlave(socket, server) {}

		// Public interfaces for protected functions we want to expose.
		std::size_t send(const char *msg,
			std::size_t len = 0,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(msg, len, flags, timeoutMs);
		}
		std::size_t send(const std::string &s,
			SendFlag flags = SendFlag::NO_SIGNAL,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::send(s, flags, timeoutMs);
		}
		std::size_t recv(char *buf,
			std::size_t len,
			RecvFlag flags = RecvFlag::NONE,
			std::size_t timeoutMs = 0) const {
			return CustomServerSlave::recv(buf, len, flags, timeoutMs);
		}
		void close() { CustomServerSlave::close(); }
		NativeSocket getNativeSocket() {
			return CustomServerSlave::getNativeSocket();
		}
	};

	class Server : private Smtp::Socket,
								 private CustomServer<Smtp::ServerSlave> {};
}

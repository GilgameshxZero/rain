// Tests compilation and instantiation of the Networking::Socket(Interface)
// inheritance suite, including the Socket options.
#include <rain/literal.hpp>
#include <rain/networking/socket-options.hpp>
#include <rain/networking/socket.hpp>

#include <cassert>
#include <iostream>

int main() {
	using namespace Rain::Literal;
	using namespace Rain::Networking;

	{
		// Instantiate a Socket with options by inheriting them from the
		// implementing interfaces.
		//
		// This is an IPv6, Stream, TCP Socket which is dual-stack.
		Socket<
			Ipv6FamilyInterface,
			StreamTypeInterface,
			TcpProtocolInterface,
			DualStackSocketOption,
			NoLingerSocketOption>
			socket;

		// Confirm that the socket options and socket F/T/P is as desired.
		std::cout << socket.family() << ' ' << socket.type() << ' '
							<< socket.protocol() << std::endl;
		assert(socket.family() == Family::INET6);
		assert(socket.type() == Type::STREAM);
		assert(socket.protocol() == Protocol::TCP);

		// Make sure subtypes are inherited correctly.
		// std::string *str = &socket;
		assert(
			static_cast<SocketFamilyInterface &>(socket).family() == Family::INET6);
		assert(static_cast<SocketTypeInterface &>(socket).type() == Type::STREAM);
		assert(
			static_cast<SocketProtocolInterface &>(socket).protocol() ==
			Protocol::TCP);
		// DGramSocketInterface *dgramSocket = &socket;

		// Make sure copy/move disabled.
		// CustomSocket socket2(socket);
		// CustomSocket socket2;
		// socket2 = socket;
		// CustomSocket socket2(std::move(socket));
		// CustomSocket socket2;
		// socket2 = std::move(socket);
	}

	return 0;
}

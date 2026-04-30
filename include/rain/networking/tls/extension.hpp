#pragma once

#include "../tcp/socket.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class Extension {
		public:
		// TODO.
		void sendWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
		void recvWith(
			Tcp::ConnectedSocketSpecInterface &) const {}
	};
}

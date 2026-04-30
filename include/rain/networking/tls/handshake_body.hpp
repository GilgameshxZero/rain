#pragma once

#include "handshake_type.hpp"

#include <cstdint>
#include <iostream>

namespace Rain::Networking::Tls {
	class HandshakeBody {
		public:
		virtual HandshakeType handshakeType() const = 0;
		virtual std::uint32_t length() const = 0;
		virtual void sendWith(std::ostream &) const = 0;

		// We need a virtual destructor since we will be
		// deleting derived instances through a base pointer.
		virtual ~HandshakeBody() {}
	};
}

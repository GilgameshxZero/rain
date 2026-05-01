#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "cipher_suite.hpp"
#include "handshake_body.hpp"
#include "protocol_version.hpp"
#include "random.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace Rain::Networking::Tls {
	class ServerKeyExchange : public HandshakeBody {
		public:
		ServerKeyExchange() {}
		ServerKeyExchange(std::istream &) {
			// TODO.
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::SERVER_KEY_EXCHANGE;
		}
		virtual std::uint32_t length() const override {
			// TODO.
			return 0;
		}
		virtual void sendWith(std::ostream &) const override {
			// TODO.
		}
	};
}

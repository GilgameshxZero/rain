#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "certificate.hpp"
#include "client_hello.hpp"
#include "handshake_body.hpp"
#include "handshake_type.hpp"
#include "plaintext_fragment.hpp"
#include "server_hello.hpp"
#include "server_key_exchange.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

namespace Rain::Networking::Tls {
	class Handshake : public PlaintextFragment {
		private:
		// Not used outside of constructor.
		std::uint16_t completeLength;
		HandshakeType type;

		HandshakeBody *makeHandshakeBody(
			HandshakeType const &type,
			auto &&...args) {
			switch (type) {
				case HandshakeType::CLIENT_HELLO:
					return new ClientHello(
						std::forward<decltype(args)>(args)...);
				case HandshakeType::SERVER_HELLO:
					return new ServerHello(
						std::forward<decltype(args)>(args)...);
				case HandshakeType::CERTIFICATE:
					return new Certificate(
						std::forward<decltype(args)>(args)...);
				case HandshakeType::SERVER_KEY_EXCHANGE:
				default:
					return new ServerKeyExchange(
						std::forward<decltype(args)>(args)...);
			}
		}

		public:
		// shared_ptr to support copy constructor.
		std::shared_ptr<HandshakeBody> body;

		// The virtual member functions disqualify Handshake as
		// an aggregate class, so we provide a constructor
		// instead.
		Handshake(HandshakeBody *body) :
			completeLength{},
			type{body->handshakeType()},
			body(body) {}
		Handshake(std::istream &stream) :
			completeLength{Algorithm::readBytes<std::uint16_t>(
				stream,
				std::endian::big)},
			type{Algorithm::readBytes<HandshakeType::Value>(
				stream,
				std::endian::big)},
			body{this->makeHandshakeBody(this->type, stream)} {}

		virtual ContentType contentType() const override {
			return ContentType::HANDSHAKE;
		}
		virtual std::uint16_t length() const override {
			return static_cast<std::uint16_t>(
				4 + this->body->length());
		};
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream,
				this->body->handshakeType(),
				std::endian::big);
			// Length is uint24_t.
			Algorithm::writeBytes(
				stream, this->body->length(), std::endian::big, 3);
			this->body->sendWith(stream);
		}
	};
};

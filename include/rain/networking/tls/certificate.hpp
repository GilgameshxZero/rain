#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "handshake_body.hpp"
#include "handshake_type.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

namespace Rain::Networking::Tls {
	class Certificate : public HandshakeBody {
		public:
		std::vector<std::string> certificates;

		Certificate() {}
		Certificate(std::istream &stream) {
			stream.get();
			stream.get();
			stream.get();
			std::int32_t certificatesLength(
				Algorithm::readBytes<std::uint32_t>(
					stream, std::endian::big, 3));
			while (stream && certificatesLength > 0) {
				auto certificateLength{
					Algorithm::readBytes<std::uint32_t>(
						stream, std::endian::big, 3)};
				this->certificates.emplace_back(
					certificateLength, '\0');
				stream.read(
					this->certificates.back().data(),
					certificateLength);
				certificatesLength -= certificateLength;
			}
		}

		virtual HandshakeType handshakeType() const override {
			return HandshakeType::CERTIFICATE;
		}
		virtual std::uint32_t length() const override {
			std::size_t certificatesLength{};
			for (auto &i : this->certificates) {
				certificatesLength += i.length() + 3;
			}
			return static_cast<std::uint32_t>(
				certificatesLength + 3);
		}
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream, this->length() - 3, std::endian::big, 1);
			for (auto &i : this->certificates) {
				Algorithm::writeBytes(
					stream,
					static_cast<std::uint32_t>(i.length() - 3),
					std::endian::big,
					1);
				stream << i;
			}
		}
	};
}

#pragma once

#include "../../algorithm/bit_manipulators.hpp"
#include "alert_description.hpp"
#include "alert_level.hpp"
#include "content.hpp"

#include <cstdint>
#include <iostream>
#include <memory>

namespace Rain::Networking::Tls {
	class Alert : public PlaintextFragment {
		public:
		AlertLevel alertLevel;
		AlertDescription alertDescription;

		Alert(
			std::uint8_t alertLevel,
			std::uint8_t alertDescription) :
			alertLevel{alertLevel},
			alertDescription{alertDescription} {}
		Alert(std::istream &stream) :
			alertLevel{
				(stream.get(),
					stream.get(),
					Algorithm::readBytes<std::uint8_t>(stream))},
			alertDescription{
				Algorithm::readBytes<std::uint8_t>(stream)} {}

		virtual ContentType contentType() const override {
			return ContentType::ALERT;
		}
		virtual std::uint16_t length() const override {
			return sizeof(this->alertLevel) +
				sizeof(this->alertDescription);
		};
		virtual void sendWith(
			std::ostream &stream) const override {
			Algorithm::writeBytes(
				stream, this->alertLevel, std::endian::big);
			Algorithm::writeBytes(
				stream, this->alertDescription, std::endian::big);
		}
	};
};

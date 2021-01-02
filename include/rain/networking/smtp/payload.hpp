#pragma once

#include "../../string.hpp"

namespace Rain::Networking::Smtp {
	class Payload {
		public:
		std::string parameter;

		Payload(const std::string &parameter = "") : parameter(parameter) {}
	};
}

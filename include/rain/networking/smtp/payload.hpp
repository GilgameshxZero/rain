#pragma once

#include "../../string.hpp"

namespace Rain::Networking::Smtp {
	class Payload {
		public:
		std::string parameter;

		Payload(std::string const &parameter = "") : parameter(parameter) {}
	};
}

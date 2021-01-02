#pragma once

#include "../../string.hpp"
#include "body.hpp"
#include "header.hpp"

namespace Rain::Networking::Http {
	class Payload {
		public:
		std::string version;
		Header header;
		Body body;

		Payload(const std::string &version = "") : version(version) {
			header["Content-Length"] = "0";
		}
	};
}

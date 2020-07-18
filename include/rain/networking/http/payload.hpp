#pragma once

#include "../../string.hpp"
#include "../socket.hpp"
#include "body.hpp"
#include "header.hpp"

#include <functional>
#include <map>

namespace Rain::Networking::Http {
	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
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

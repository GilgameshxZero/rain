#pragma once

#include "../../string.hpp"

#include <functional>
#include <map>

namespace Rain::Networking::Smtp {
	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class Payload {
		public:
		std::string parameter;

		Payload(const std::string &parameter = "") : parameter(parameter) {}
	};

	class Request : public Payload {
		public:
		typedef std::function<void(Request *)> Handler;

		std::string verb;

		Request(const std::string &verb = "EHLO", const std::string &parameter = "")
				: Payload(parameter), verb(verb) {}
	};
	class Response : public Payload {
		public:
		typedef std::function<void(Response *)> Handler;

		std::size_t code;
		std::vector<std::vector<std::string>> extensions;

		Response(std::size_t code = 250, const std::string &parameter = "")
				: Payload(parameter), code(code) {}
	};
}

#pragma once

#include "payload.hpp"

namespace Rain::Networking::Http {
	class Request : public Payload {
		public:
		typedef std::function<void(Request *)> Handler;

		std::string method, path, query, fragment;

		Request(const std::string &method = "GET",
			const std::string &path = "/",
			const std::string &query = "",
			const std::string &fragment = "",
			const std::string &version = "1.1")
				: Payload(version),
					method(method),
					path(path),
					query(query),
					fragment(fragment) {}
	};
	class Response : public Payload {
		public:
		typedef std::function<void(Response *)> Handler;

		std::size_t statusCode;
		std::string status;

		Response(std::size_t statusCode = 200,
			const std::string &status = "OK",
			const std::string &version = "1.1")
				: Payload(version), statusCode(statusCode), status(status) {}
	};
}

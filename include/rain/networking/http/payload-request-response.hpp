#pragma once

#include "../../string.hpp"
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

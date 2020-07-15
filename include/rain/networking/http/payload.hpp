#pragma once

#include "../../string.hpp"
#include "../socket.hpp"

#include <functional>
#include <map>

namespace Rain::Networking::Http {
	// Requests and responses are passed to handlers based on whether they're a
	// part of the client or server.
	class Payload {
		public:
		typedef std::function<std::size_t(char **)> BodyGetter;

		std::string version;
		std::map<std::string, std::string> headers;

		// Load more body. Returns number of characters retrieved. Sets parameter to
		// point to location of the newly retrieved body (or body to send). Returns
		// 0 to signal the end of the body.
		inline static const BodyGetter NO_BODY = [](char **) { return 0; };
		BodyGetter getBody = NO_BODY;

		Payload(const std::string &version = "") : version(version) {
			this->headers["content-length"] = "0";
		}

		// Convenience functions for setting the body.
		// The parameter passed in must persist until the body is sent.
		void setBody(const char *body) {
			std::size_t bodyLen = strlen(body);
			this->headers["content-length"] = std::to_string(bodyLen);

			// Need to capture by copy; references break when trying to cast to
			// pointer again.
			this->getBody = [this, body, bodyLen](char **retBody) {
				*retBody = const_cast<char *>(body);
				
				// When getBody is set, it may damage the lambda enclosure. So do
				// everything before setting this. Same goes for all other lambdas.
				std::size_t ret = bodyLen;
				this->getBody = NO_BODY;
				return ret;
			};
		}
		void setBody(const std::string &body) {
			this->headers["content-length"] = std::to_string(body.length());
			this->getBody = [this, body](char **retBody) {
				*retBody = const_cast<char *>(body.c_str());
				std::size_t ret = body.length();
				this->getBody = NO_BODY;
				return ret;
			};
		}
	};
}

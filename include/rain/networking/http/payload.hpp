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

		Payload(std::string const &version = "1.1") : version(version) {}

		protected:
		// Set Content-Length if not set and body is static.
		void checkSetContentLength() {
			if (this->header.find("Content-Length") == this->header.end() &&
				this->body.getIsStatic()) {
				this->header["Content-Length"] = std::to_string(this->body.getLength());
			}
		}
	};
}

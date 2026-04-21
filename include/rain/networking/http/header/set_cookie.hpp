// Type for the Set-Cookie header.
#pragma once

#include "../../../string/string.hpp"

#include <unordered_map>

namespace Rain::Networking::Http::Header {
	class SetCookie {
		public:
		std::string value;
		std::unordered_map<
			std::string,
			std::string,
			String::CaseAgnosticHash,
			String::CaseAgnosticEqual>
			attributes;

		SetCookie(
			std::string const &value,
			std::unordered_map<
				std::string,
				std::string,
				String::CaseAgnosticHash,
				String::CaseAgnosticEqual> const &attributes = {})
				: value(value), attributes(attributes) {}
	};
}

// Type for the Set-Cookie header.
#pragma once

#include "../../../string/string.hpp"

#include <unordered_map>

namespace Rain::Networking::Http::Header {
	class Authorization {
		public:
		// Helper to decode a base-64 encoded credentials string as part of the
		// Basic scheme.
		static std::pair<std::string, std::string> decodeBasicCredentials(
			std::string const &credentials) {
			std::string raw{Rain::String::Base64::decode(credentials)};
			std::size_t separator{raw.find(':')};
			return {raw.substr(0, separator - 1), raw.substr(separator + 1)};
		}
		static std::string encodeBasicCredentials(
			std::string const &username,
			std::string const &password) {
			std::string raw{username + ":" + password};
			return Rain::String::Base64::encode(raw);
		}

		std::string scheme;
		std::unordered_map<
			std::string,
			std::string,
			String::CaseAgnosticHash,
			String::CaseAgnosticEqual>
			parameters;

		Authorization(
			std::string const &scheme = "",
			std::unordered_map<
				std::string,
				std::string,
				String::CaseAgnosticHash,
				String::CaseAgnosticEqual> const &parameters = {})
				: scheme(scheme), parameters(parameters) {}
	};
}

// Represents an email example@example.com from RFC 5322.
#pragma once

#include "../../string/string.hpp"

#include <iostream>

namespace Rain::Networking::Smtp {
	// Represents an email example@example.com from RFC 5322 Section 3.4.1.
	class Mailbox {
		public:
		enum class Error { EMPTY_NAME_OR_DOMAIN = 1 };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Smtp::Mailbox";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::EMPTY_NAME_OR_DOMAIN:
						return "Empty name or domain supplied to mailbox.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		std::string name;
		std::string domain;

		// name@domain notation. Throws if either part is empty.
		Mailbox(std::string const &str = "")
				: name(str.substr(0, str.find('@'))),
					domain(str.substr(std::min(str.find('@'), str.length() - 1) + 1)) {
			if (name.empty() || domain.empty()) {
				throw Exception(Error::EMPTY_NAME_OR_DOMAIN);
			}
		}

		Mailbox(std::string const &name, std::string const &domain)
				: name(name), domain(domain) {}

		operator std::string() const noexcept {
			return this->name + "@" + this->domain;
		}

		// Equality for unordered_ types.
		bool operator==(Mailbox const &other) const noexcept {
			return this->name == other.name && this->domain == other.domain;
		}
	};

	// Hash Mailbox for unordered_set and unordered_map.
	struct HashMailbox {
		std::size_t operator()(Mailbox const &mailbox) const {
			return std::hash<std::string>()(mailbox.operator std::string());
		}
	};
}

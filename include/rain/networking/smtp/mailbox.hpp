// Represents an email example@example.com from RFC 5322.
#pragma once

#include "../../string/string.hpp"

#include <iostream>

namespace Rain::Networking::Smtp {
	// Represents an email example@example.com from RFC 5322 Section 3.4.1.
	class Mailbox {
		public:
		std::string name;
		std::string domain;

		// name@domain notation. Default construction leaves both empty.
		Mailbox(std::string const &str = "")
				: name(str.substr(0, str.find('@'))),
					domain(str.substr(std::min(str.find('@'), str.length() - 1) + 1)) {}

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

inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Smtp::Mailbox const &mailbox) {
	return stream << static_cast<std::string>(mailbox);
}
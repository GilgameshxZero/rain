// Host, Node, Service implement representations of the node/service
// specification for an internet address.
#pragma once

#include "../literal.hpp"
#include "../string.hpp"

namespace Rain::Networking {
	// Host, Node, Service implement representations of the node/service
	// specification for an internet address.
	//
	// A collection of string node, service, with some helper functions bundled
	// in.
	class Host {
		public:
		// Empty node will be converted to nullptr when passed to gai, binding to
		// all available interfaces. Empty service will be converted to nullptr when
		// passed to gai, binding to random port (port 0). Both cannot be empty when
		// used with gai.
		std::string node, service;

		// Host:Service notation.
		Host(std::string const &str = "")
				: node(str.substr(0, str.find(':'))),
					service(str.substr(std::min(str.find(':'), str.length() - 1) + 1)) {}
		Host(char const *str) : Host(std::string(str)) {}

		// Constructor variants.
		Host(std::string const &node, std::size_t service)
				: node(node), service(std::to_string(service)) {}
		Host(std::string const &node, std::string const &service)
				: node(node), service(service) {}

		// Default copy & move semantics. Copy semantics must be explicitly defined
		// if move is also defined.
		Host(Host const &) = default;
		Host &operator=(Host const &) = default;
		Host(Host &&) = default;
		Host &operator=(Host &&) = default;

		// Convert to string.
		std::string asStr() const {
			// Host:Service notation only contains a ':' if Service is not empty, or
			// if both are empty.
			return this->service.empty() ? (this->node.empty() ? ":" : this->node)
																	 : this->node + ":" + this->service;
		}
		operator std::string() const { return this->asStr(); }

		// Operators.
		bool operator==(Host const &other) {
			return this->node == other.node && this->service == other.service;
		}
		bool operator!=(Host const &other) { return !this->operator==(other); }
	};
}

// User-defined operators for outputting hostnames.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Host const &host) {
	return stream << host.asStr();
}

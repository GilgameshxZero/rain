// Host, Node, Service implement representations of the node/service
// specification for an internet address.
#pragma once

#include "../literal.hpp"
#include "../string.hpp"

namespace Rain::Networking {
	class Host {
		public:
		// Empty node will be converted to nullptr when passed to gai, binding to
		// all available interfaces. Empty service will be converted to nullptr when
		// passed to gai, binding to random port (port 0).
		std::string node, service;

		// Host:Service notation.
		Host(std::string const &str = "")
				: node(str.substr(0, str.find(':'))),
					service(str.substr(std::min(str.find(':'), str.length() - 1) + 1)) {}

		Host(std::string const &node, std::size_t service)
				: node(node), service(std::to_string(service)) {}
		Host(std::string const &node, std::string const &service)
				: node(node), service(service) {}

		operator std::string() const noexcept {
			// Host:Service notation only contains a ':' if Service is not empty, or
			// if both are empty.
			return this->service.empty() ? (this->node.empty() ? ":" : this->node)
																	 : this->node + ":" + this->service;
		}
	};
}

// User-defined operators for outputting hostnames.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Host const &host) {
	return stream << static_cast<std::string>(host);
}

#pragma once

#include "../string.hpp"

namespace Rain::Networking {
	// Any values passed in are copied, so there is no need to consider memory
	// management.
	class Host {
		public:
		class Node {
			public:
			// Passing "" is the same as passing a NULL host, most useful for binding on localhost.
			Node(std::string const &node) : node(node) {}
			Node(char const *node) : node(node == NULL ? "" : node) {}

			// An int constructor is included for compatibility for constructing with
			// NULL.
			Node(int node) : node(node == 0 ? "" : std::to_string(node)) {}

			// Passing NULL as the node to getaddrinfo is consistent for binding to
			// localhost. Connect directly with localhost when attempting a
			// connection.
			char const *getCStr() const noexcept {
				return this->node.empty() ? NULL : this->node.c_str();
			}

			private:
			std::string node;
		};
		class Service {
			public:
			Service(std::string const &service) : service(service) {}
			Service(char const *service) : service(service) {}
			Service(std::size_t service) : service(std::to_string(service)) {}
			Service(int service) : service(std::to_string(service)) {}

			char const *getCStr() const noexcept { return this->service.c_str(); }

			private:
			std::string service;
		};

		Node node;
		Service service;

		template <typename NodeType, typename ServiceType>
		Host(NodeType node, ServiceType service) : node(node), service(service) {}
	};
}

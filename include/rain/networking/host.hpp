#pragma once

#include "../string.hpp"

namespace Rain::Networking {
	// Any values passed in are copied, so there is no need to consider memory
	// management.
	class Host {
		public:
		// Pass "localhost" to bind & listen for any incoming connections, or to
		// connect to localhost addresses. This is more reliable cross-platform than
		// "*" or "127.0.0.1" or "0.0.0.0".
		class Node {
			public:
			Node(const std::string &node) : node(node) {}
			Node(const char *node) : node(node == NULL ? "localhost" : node) {}
			Node(int node) : node(node == 0 ? "localhost" : std::to_string(node)) {}

			const char *getCStr() const noexcept {
				return this->node == "localhost" ? NULL : this->node.c_str();
			}

			private:
			std::string node;
		};
		class Service {
			public:
			Service(const std::string &service) : service(service) {}
			Service(const char *service) : service(service) {}
			Service(std::size_t service) : service(std::to_string(service)) {}
			Service(int service) : service(std::to_string(service)) {}

			const char *getCStr() const noexcept { return this->service.c_str(); }

			private:
			std::string service;
		};

		Node node;
		Service service;

		template <typename NodeType, typename ServiceType>
		Host(NodeType node, ServiceType service) : node(node), service(service) {}
	};
}

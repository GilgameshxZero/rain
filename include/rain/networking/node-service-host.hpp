#pragma once

#include "../string.hpp"

namespace Rain::Networking {
	class Host {
		public:
		class Node {
			public:
			Node(const std::string &node) : node(node) {}
			Node(const char *node) : node(node) {}

			const char *getCStr() const noexcept { return this->node.c_str(); }

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

		const Node node;
		const Service service;

		template <typename NodeType, typename ServiceType>
		Host(NodeType node, ServiceType service) : node(node), service(service) {}
	};
}

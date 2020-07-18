#pragma once

#include "../string.hpp"

namespace Rain::Networking {
	class Host {
		public:
		class Node {
			public:
			Node(const char *node) : node(node) {}

			const char *getCStr() const noexcept { return this->node; }

			private:
			const char *node;
		};
		class Service {
			public:
			Service(const char *service) : service(service) {}
			Service(std::size_t service) : service(std::to_string(service)) {}

			const char *getCStr() const noexcept { return this->service.c_str(); }

			private:
			std::string service;
		};

		const Node node;
		const Service service;

		Host(const Node &node, const Service &service)
				: node(node), service(service) {}
		Host(const char *node, const char *service)
				: node(node), service(service) {}
		Host(const char *node, const std::string &service)
				: node(node), service(service.c_str()) {}
		Host(const char *node, std::size_t service)
				: node(node), service(service) {}
	};
}

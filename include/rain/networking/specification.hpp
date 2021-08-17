// Implements enums for protocol/address family, socket type, socket protocol,
// and other networking-related specification flags used in multiple locations.
#pragma once

#include "native-socket.hpp"

namespace Rain::Networking {
	// Under the assumption that AF/PF map to the same values.
	enum class Family { UNSPEC = AF_UNSPEC, INET = AF_INET, INET6 = AF_INET6 };
	enum class Type {
		ANY = 0,
		STREAM = SOCK_STREAM,
		DGRAM = SOCK_DGRAM,
		RAW = SOCK_RAW
	};
	enum class Protocol {
		ANY = 0,
		ICMP = IPPROTO_ICMP,
		TCP = IPPROTO_TCP,
		UDP = IPPROTO_UDP,
		ICMPV6 = IPPROTO_ICMPV6
	};

	// Sockets derive from F/T/P classes to configure their F/T/P.
	//
	// Anything which expects F/T/P should inherit the interfaces virtually.
	// Otherwise, declaring either F/T/P should be done by inheriting a concrete
	// F/T/P class.
	class SocketFamilyInterface {
		public:
		virtual Family family() const noexcept = 0;
	};
	class SocketTypeInterface {
		public:
		virtual Type type() const noexcept = 0;
	};
	class SocketProtocolInterface {
		public:
		virtual Protocol protocol() const noexcept = 0;
	};

	// F-Spec classes.
	class Ipv4FamilyInterface : virtual public SocketFamilyInterface {
		public:
		virtual Family family() const noexcept final override {
			return Family::INET;
		}
	};
	class Ipv6FamilyInterface : virtual public SocketFamilyInterface {
		public:
		virtual Family family() const noexcept final override {
			return Family::INET6;
		}
	};

	// T-Spec classes.
	class StreamTypeInterface : virtual public SocketTypeInterface {
		public:
		virtual Type type() const noexcept final override { return Type::STREAM; }
	};
	class DGramTypeInterface : virtual public SocketTypeInterface {
		public:
		virtual Type type() const noexcept final override { return Type::DGRAM; }
	};
	class RawTypeInterface : virtual public SocketTypeInterface {
		public:
		virtual Type type() const noexcept final override { return Type::RAW; }
	};

	// P-Spec classes.
	class IcmpProtocolInterface : virtual public SocketProtocolInterface {
		public:
		virtual Protocol protocol() const noexcept final override {
			return Protocol::ICMP;
		}
	};
	class TcpProtocolInterface : virtual public SocketProtocolInterface {
		public:
		virtual Protocol protocol() const noexcept final override {
			return Protocol::TCP;
		}
	};
	class UdpProtocolInterface : virtual public SocketProtocolInterface {
		public:
		virtual Protocol protocol() const noexcept final override {
			return Protocol::UDP;
		}
	};
	class Icmpv6ProtocolInterface : virtual public SocketProtocolInterface {
		public:
		virtual Protocol protocol() const noexcept final override {
			return Protocol::ICMPV6;
		}
	};
}

// Stream operators for F/T/P.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Family family) {
	using namespace Rain::Networking;
	switch (family) {
		case Family::UNSPEC:
		default:
			return stream << "UNSPEC";
			break;
		case Family::INET:
			return stream << "INET";
			break;
		case Family::INET6:
			return stream << "INET6";
			break;
	}
}
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Type type) {
	using namespace Rain::Networking;
	switch (type) {
		case Type::ANY:
		default:
			return stream << "ANY";
			break;
		case Type::STREAM:
			return stream << "STREAM";
			break;
		case Type::DGRAM:
			return stream << "DGRAM";
			break;
		case Type::RAW:
			return stream << "RAW";
			break;
	}
}
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Protocol protocol) {
	using namespace Rain::Networking;
	switch (protocol) {
		case Protocol::ANY:
		default:
			return stream << "ANY";
			break;
		case Protocol::ICMP:
			return stream << "ICMP";
			break;
		case Protocol::TCP:
			return stream << "TCP";
			break;
		case Protocol::UDP:
			return stream << "UDP";
			break;
		case Protocol::ICMPV6:
			return stream << "ICMPV6";
			break;
	}
}

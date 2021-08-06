// Implements enums for protocol/address family, socket type, socket protocol,
// and other networking-related specification flgas used in multiple locations.
#pragma once

#include "native-socket.hpp"

namespace Rain::Networking::Specification {
	// Only defins commonly-used constants. Expand if necessary.
	enum class AddressFamily {
		DEFAULT = -1,
		UNSPEC = AF_UNSPEC,	 // 0.

		INET = AF_INET,

		INET6 = AF_INET6
	};

	// On most platforms, ProtocolFamily can be used interchangeably with
	// AdressFamily. Only defins commonly-used constants. Expand if necessary.
	enum class ProtocolFamily {
		DEFAULT = -1,
		UNSPEC = PF_UNSPEC,	 // 0.

		INET = PF_INET,

		INET6 = PF_INET6
	};

	// Only defins commonly-used constants. Expand if necessary.
	enum class SocketType {
		DEFAULT = -1,
		ANY = 0,

		STREAM = SOCK_STREAM,
	};

	// Only defins commonly-used constants. Expand if necessary.
	enum class SocketProtocol {
		DEFAULT = -1,
		ANY = 0,

		TCP = IPPROTO_TCP,

		UDP = IPPROTO_UDP
	};

	// Converts PF/AF.
	AddressFamily PfToAf(ProtocolFamily pf) {
		return static_cast<AddressFamily>(pf);
	}
	ProtocolFamily AfToPf(AddressFamily af) {
		return static_cast<ProtocolFamily>(af);
	}

	// A bundle of specifications, on which PF/AF can be retrieved at will.
	class Specification {
		private:
		AddressFamily af;
		ProtocolFamily pf;
		SocketType st;
		SocketProtocol sp;

		public:
		Specification(
			AddressFamily af = AddressFamily::DEFAULT,
			SocketType st = SocketType::DEFAULT,
			SocketProtocol sp = SocketProtocol::DEFAULT) noexcept
				: af(af), pf(AfToPf(af)), st(st), sp(sp) {}
		Specification(ProtocolFamily pf, SocketType st, SocketProtocol sp) noexcept
				: af(PfToAf(pf)), pf(pf), st(st), sp(sp) {}

		// Construct from an original and a proposed, replacing defaults in proposed
		// with original.
		Specification(
			Specification const &original,
			Specification const &proposed) noexcept
				: Specification(
						proposed.getAddressFamily() == AddressFamily::DEFAULT
							? original.getAddressFamily()
							: proposed.getAddressFamily(),
						proposed.getSocketType() == SocketType::DEFAULT
							? original.getSocketType()
							: proposed.getSocketType(),
						proposed.getSocketProtocol() == SocketProtocol::DEFAULT
							? original.getSocketProtocol()
							: proposed.getSocketProtocol()) {}

		void setFamily(AddressFamily af) noexcept {
			this->af = af;
			this->pf = AfToPf(af);
		}
		void setFamily(ProtocolFamily pf) noexcept {
			this->pf = pf;
			this->af = PfToAf(pf);
		}
		void setSocketType(SocketType st) noexcept { this->st = st; }
		void setSocketProtocol(SocketProtocol sp) noexcept { this->sp = sp; }

		AddressFamily getAddressFamily() const noexcept { return this->af; }
		ProtocolFamily getProtocolFamily() const noexcept { return this->pf; }
		SocketType getSocketType() const noexcept { return this->st; }
		SocketProtocol getSocketProtocol() const noexcept { return this->sp; }
	};
}

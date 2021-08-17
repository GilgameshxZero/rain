// These classes allow applying properties to an underlying Socket by inheriting
// them. All of these classes have constructors which should be called after
// that of a resource-owning Socket constructor.
//
// To ensure that nativeSocket() is overridden at construction, these classes
// must inherit a resource-owning Socket constructor. Multiple options may be
// set by stacking templates; since the class generated from one option template
// is a valid Socket for another option template.
#pragma once

#include "socket.hpp"

namespace Rain::Networking {
	class SocketOptionInterface : virtual public SocketInterface {
		protected:
		// Code sharing.
		template <typename Value>
		static void setOption(
			NativeSocket nativeSocket,
			int level,
			int name,
			Value const &value) {
			validateSystemCall(setsockopt(
				nativeSocket,
				level,
				name,
#ifdef RAIN_PLATFORM_WINDOWS
				reinterpret_cast<char const *>(&value),
#else
				reinterpret_cast<void const *>(&value),
#endif
				sizeof(value)));
		}
	};

	class NoLingerSocketOptionInterface : virtual public SocketOptionInterface {};

	// Don't linger. Thus causes any close on the socket to abort the connection
	// immediately and doesn't let the Socket idle areound in the kernel.
	template <typename Socket>
	class NoLingerSocketOption : public Socket,
															 virtual public NoLingerSocketOptionInterface {
		using Socket::Socket;

		private:
		class _NoLingerSocketOption {
			public:
			_NoLingerSocketOption(NoLingerSocketOption *that) {
				SocketOptionInterface::setOption(
					that->nativeSocket(), SOL_SOCKET, SO_LINGER, linger{1, 0});
			}
		};
		_NoLingerSocketOption _noLingerSocketOption = _NoLingerSocketOption(this);

		// No linger can only be set once.
		virtual void alreadyNoLingerSocketOption() final {}
	};

	class DualStackSocketOptionInterface : virtual public SocketOptionInterface,
																				 virtual public Ipv6FamilyInterface {};

	// An IPv6-protocol Socket is declared dual-stack by setting this option.
	//
	// Enforces IPv6 family; otherwise, a compile-time error for ambiguous
	// resolution is generated.
	template <typename Socket>
	class DualStackSocketOption : public Socket,
																virtual public DualStackSocketOptionInterface {
		using Socket::Socket;

		private:
		class _DualStackSocketOption {
			public:
			_DualStackSocketOption(DualStackSocketOption *that) {
				SocketOptionInterface::setOption(
					that->nativeSocket(),
					IPPROTO_IPV6,
					IPV6_V6ONLY,
#ifdef RAIN_PLATFORM_WINDOWS
					DWORD{0});
#else
					int{0});
#endif
			}
		};
		_DualStackSocketOption _dualStackSocketOption =
			_DualStackSocketOption(this);

		virtual void alreadyIpv6OnlySocketOption() final {}
	};

	class Ipv6OnlySocketOptionInterface : virtual public SocketOptionInterface,
																				virtual public Ipv6FamilyInterface {};

	// An IPv6-protocol Socket is declared single-stack. This is usually, but not
	// guaranteed, to be the default on most systems.
	//
	// Enforces IPv6 family; otherwise, a compile-time error for ambiguous
	// resolution is generated.
	template <typename Socket>
	class Ipv6OnlySocketOption : public Socket,
															 virtual public Ipv6OnlySocketOptionInterface {
		using Socket::Socket;

		private:
		class _Ipv6OnlySocketOption {
			public:
			_Ipv6OnlySocketOption(Ipv6OnlySocketOption *that) {
				SocketOptionInterface::setOption(
					that->nativeSocket(),
					IPPROTO_IPV6,
					IPV6_V6ONLY,
#ifdef RAIN_PLATFORM_WINDOWS
					DWORD{1});
#else
					int{1});
#endif
			}
		};
		_Ipv6OnlySocketOption _ipv6OnlySocketOption = _Ipv6OnlySocketOption(this);

		// No linger can only be set once.
		virtual void alreadyIpv6OnlySocketOption() final {}
	};

	class ReuseAddressSocketOptionInterface
			: virtual public SocketOptionInterface {};

	// There are many concerns related to reusing of addresses:
	// <https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ>.
	template <typename Socket>
	class ReuseAddressSocketOption
			: public Socket,
				virtual public ReuseAddressSocketOptionInterface {
		using Socket::Socket;

		private:
		class _ReuseAddressSocketOption {
			public:
			_ReuseAddressSocketOption(ReuseAddressSocketOption *that) {
				SocketOptionInterface::setOption(
					that->nativeSocket(),
					SOL_SOCKET,
					SO_REUSEADDR,
#ifdef RAIN_PLATFORM_WINDOWS
					ULONG{1});
#else
					int{1});
#endif
			}
		};
		_ReuseAddressSocketOption _reuseAddressSocketOption =
			_ReuseAddressSocketOption(this);

		virtual void alreadyReuseAddressSocketOption() final {}
	};
}

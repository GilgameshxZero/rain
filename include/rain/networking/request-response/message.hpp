// Messages contain shared functionality of R/Rs.
#pragma once

#include "../../error/exception.hpp"

#include <istream>
#include <ostream>

namespace Rain::Networking::RequestResponse {
	// Messages contain shared functionality of R/Rs.
	//
	// The R/R MessageInterface simply disallows sending/recving twice on the same
	// object.
	//
	// Subprotocol MessageInterfaces should inherit from MessageInterface, and the
	// subprotocol R/Rs should inherit the subprotocol R/RInterface, which should
	// template inherit the subprotocol MessageInterface.
	class MessageInterface {
		private:
		typedef MessageInterface Interface;

		public:
		// Default constructor explicitly declared here.
		MessageInterface() = default;
		virtual ~MessageInterface() = default;

		// Move constructor must be defined.
		MessageInterface(MessageInterface &&) = default;

		protected:
		// To avoid the call super pattern, subclasses should override these
		// implementation functions instead, with the correct tags for multi-level
		// inheritance.
		template <typename>
		struct Tag {};

		// Alias current class tag with InterfaceTag.
		typedef Tag<Interface> InterfaceTag;

		private:
		virtual void sendWithImpl(InterfaceTag, std::ostream &) = 0;
		virtual void recvWithImpl(InterfaceTag, std::istream &) = 0;

		public:
		// Always succeeds or throws if failure/timeout (via maxRecvIdleDuration or
		// sendOnceTimeoutDuration).
		void sendWith(std::ostream &stream) {
			this->sendWithImpl(InterfaceTag(), stream);
		}
		void recvWith(std::istream &stream) {
			this->recvWithImpl(InterfaceTag(), stream);
		}
	};
}

// Free functions for inputting/outputting from a normal iostream. Should be
// overridden by pp-chain overloads, as these do not provide pp-chain benefits.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::RequestResponse::MessageInterface &message) {
	message.sendWith(stream);
	return stream;
}
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Networking::RequestResponse::MessageInterface &message) {
	message.recvWith(stream);
	return stream;
}

// rvalue reference overloads for inline-constructed R/R.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::RequestResponse::MessageInterface &&message) {
	return stream << message;
}

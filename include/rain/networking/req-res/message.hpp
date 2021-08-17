// Messages contain shared functionality of R/Rs.
#pragma once

#include <iostream>

namespace Rain::Networking::ReqRes {
	// Messages contain shared functionality of R/Rs.
	class MessageInterface {
		public:
		// Default constructor explicitly declared here.
		MessageInterface() = default;
		virtual ~MessageInterface() = default;

		// No copy, move construct OK.
		MessageInterface(MessageInterface const &) = delete;
		MessageInterface &operator=(MessageInterface const &) = delete;
		MessageInterface(MessageInterface &&) = default;
		MessageInterface &operator=(MessageInterface &&) = delete;

		// Always succeeds or throws if failure/timeout. Override & call super for
		// pp-chaining at send/recv-site; that is, pp-chains here will activate with
		// send/recv on any i/ostream.
		virtual void sendWith(std::ostream &stream) = 0;
		virtual void recvWith(std::istream &stream) = 0;
	};

	class Message : virtual public MessageInterface {
		using MessageInterface::MessageInterface;
	};
}

// Free functions for inputting/outputting from a normal iostream. Should be
// overridden by pp-chain overloads, as these do not provide pp-chain benefits.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::ReqRes::MessageInterface &message) {
	message.sendWith(stream);
	return stream;
}
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Networking::ReqRes::MessageInterface &message) {
	message.recvWith(stream);
	return stream;
}

// rvalue reference overloads for inline-constructed R/R.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::ReqRes::MessageInterface &&message) {
	return stream << message;
}

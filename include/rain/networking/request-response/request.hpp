// Abstract Request for R/R Sockets.
#pragma once

#include <istream>
#include <ostream>

namespace Rain::Networking::RequestResponse {
	// Abstract Request for R/R Sockets. See MessageInterface for details on
	// intended usage.
	template <typename ProtocolMessage>
	class RequestInterface : public ProtocolMessage {
		public:
		typedef ProtocolMessage Message;

		private:
		typedef RequestInterface<Message> Interface;

		protected:
		typedef typename Message::InterfaceTag SuperTag;
		typedef typename Message::template Tag<Interface> InterfaceTag;

		public:
		// Relay constructor arguments to protocol Message.
		template <typename... MessageArgs>
		RequestInterface(MessageArgs &&...args)
				: Message(std::forward<MessageArgs>(args)...) {}

		// Allow default constrcutor for subclasses which wish to call recvWith
		// themselves (subclass overrides are not called from the constructor of a
		// base class).
		RequestInterface() = default;

		// Move constructor must be defined.
		RequestInterface(RequestInterface &&) = default;

		private:
		// Always succeeds on return or throws on failure/timeout (via
		// maxRecvIdleDuration or sendOnceTimeoutDuration).
		virtual void sendWithImpl(InterfaceTag, std::ostream &) = 0;
		virtual void recvWithImpl(InterfaceTag, std::istream &) = 0;

		virtual void sendWithImpl(SuperTag, std::ostream &stream) final override {
			this->sendWithImpl(InterfaceTag(), stream);
		}
		virtual void recvWithImpl(SuperTag, std::istream &stream) final override {
			this->recvWithImpl(InterfaceTag(), stream);
		}
	};
}

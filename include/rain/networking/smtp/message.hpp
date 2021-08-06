// Shared functionality between Smtp R/R.
#pragma once

#include "../../string.hpp"
#include "../request-response/message.hpp"

namespace Rain::Networking::Smtp {
	// Messages must be able to be sent multiple times.
	class MessageInterface : public RequestResponse::MessageInterface {
		private:
		// SuperInterface aliases the superclass.
		typedef RequestResponse::MessageInterface SuperInterface;

		// Interface aliases this class.
		typedef MessageInterface Interface;

		protected:
		// Tag aliases for sendWith/recvWith convenience.
		typedef typename SuperInterface::InterfaceTag SuperTag;
		typedef typename SuperInterface::template Tag<Interface> InterfaceTag;

		public:
		// Constructor arguments passed in from R/R. Move construct heavy arguments
		// with rvalue references.
		MessageInterface() {}

		// Must enable move construct for PreResponse.
		MessageInterface(MessageInterface &&other)
				: SuperInterface(std::move(other)) {}

		// PP chain elements.
		public:
		// Implementations of sendWith/recvWith from R/R MessageInterface.
		private:
		// Provided for subclass override.
		virtual void sendWithImpl(InterfaceTag, std::ostream &) = 0;
		virtual void recvWithImpl(InterfaceTag, std::istream &) = 0;

		// Overrides for Super versions implement protocol behavior.
		//
		// Essentially no-op to allow for tight integration with R/R.
		virtual void sendWithImpl(SuperTag, std::ostream &stream) final override {
			this->sendWithImpl(InterfaceTag(), stream);
		}
		virtual void recvWithImpl(SuperTag, std::istream &stream) final override {
			this->recvWithImpl(InterfaceTag(), stream);
		}
	};
	typedef MessageInterface Message;
}

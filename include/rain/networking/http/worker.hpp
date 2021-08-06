// HTTP Worker specialization.
#pragma once

#include "../request-response/worker.hpp"
#include "socket.hpp"

#include <list>
#include <optional>
#include <regex>

namespace Rain::Networking::Http {
	// HTTP Worker specialization.
	template <typename ProtocolSocket>
	class WorkerInterface
			: public RequestResponse::WorkerInterface<ProtocolSocket> {
		public:
		typedef ProtocolSocket Socket;

		// Alias Socket templates.
		using typename Socket::Request;
		using typename Socket::Response;
		using typename Socket::Clock;
		using typename Socket::Duration;
		using typename Socket::Message;

		private:
		// SuperInterface aliases the superclass.
		typedef RequestResponse::WorkerInterface<Socket> SuperInterface;

		public:
		template <typename TagType>
		using Tag = typename SuperInterface::template Tag<TagType>;

		// Interface aliases this class.
		typedef WorkerInterface<Socket> Interface;

		// Same constructor as R/R Worker.
		template <typename... SocketArgs>
		WorkerInterface(
			Resolve::AddressInfo const &addressInfo,
			Networking::Socket &&socket,
			std::size_t recvBufferLen = 1_zu << 10,
			std::size_t sendBufferLen = 1_zu << 10,
			Duration maxRecvIdleDuration = 60s,
			Duration sendOnceTimeoutDuration = 60s,
			SocketArgs &&...args)
				: SuperInterface(
						addressInfo,
						std::move(socket),
						recvBufferLen,
						sendBufferLen,
						maxRecvIdleDuration,
						sendOnceTimeoutDuration,
						std::forward<SocketArgs>(args)...) {}
		~WorkerInterface() {}

		WorkerInterface(WorkerInterface const &) = delete;
		WorkerInterface &operator=(WorkerInterface const &) = delete;

		// RequestHandlers return a variant of a Response with an additional
		// parameter specifying if the connection should be closed after sending it.
		//
		// PreResponse = pre-processing Response. Cannot derive from non-virtual
		// destructor std::optional.
		class PreResponse {
			private:
			std::optional<Response> optional;

			public:
			// If true, closes after the Response is sent, or aborts with no Response.
			bool toClose;

			// Default constructor means that no response is created and the
			// connection is aborted.
			PreResponse() : optional(), toClose(true) {}

			// A non-default constructor requires at least a status code.
			template <typename... MessageArgs>
			PreResponse(
				StatusCode statusCode,
				Headers &&headers = Headers(),
				Body &&body = Body(),
				std::string const &reasonPhrase = "",
				Version version = Version::_1_1,
				bool toClose = false,
				MessageArgs &&...args)
					: optional(
							std::in_place,
							statusCode,
							std::move(headers),
							std::move(body),
							reasonPhrase,
							version,
							std::forward<MessageArgs>(args)...),
						toClose(toClose) {}
			PreResponse(PreResponse const &) = delete;
			PreResponse &operator=(PreResponse const &) = delete;
			PreResponse(PreResponse &&other)
					: optional(std::move(other.optional)), toClose(other.toClose) {}

			Response &value() { return this->optional.value(); }

			constexpr explicit operator bool() const noexcept {
				return this->optional.operator bool();
			}
		};

		// RequestHandlers will be matched by regex, and should return the Response
		// to send back. If this->isValid is false afterwards, the Response will not
		// be sent back.
		//
		// Should not throw.
		typedef std::function<PreResponse(Request &, std::smatch const &)>
			RequestHandler;

		private:
		// Default match chain elements.
		//
		// Match chain element responds with NOT_FOUND.
		PreResponse matchAll(Request &req) {
			return {StatusCode::NOT_FOUND, {}, {}, "", req.version};
		}

		// Subclasses define behavior primarily by extending the preprocessor,
		// regex-match, and postprocessor chains.
		//
		// The preprocessor chain preprocesses incoming Requests. The regex-match
		// chain attempts to match target with a regex and runs a handler if
		// matched, producing a WResponse. The postprocessor chain processes
		// outgoing Responses before they are sent.
		//
		// Chains are implemented via polymorphism, and are implemented via pp
		// chains from R/R Socket.

		private:
		// Chain subclass functions are initially empty and to be overridden.
		//
		// Return an empty std::optional if the target was not matched. Return an
		// empty PreResponse if the target was matched, but the connection is to be
		// aborted. Return a PreResponse with a Response if a response is to be
		// sent.
		virtual std::optional<PreResponse> chainMatchTargetImpl(
			Tag<Interface>,
			Request &) {
			return {};
		}

		std::optional<PreResponse> chainMatchTarget(Request &req) {
			std::optional<PreResponse> prevChainResponse =
				this->chainMatchTargetImpl(Tag<Interface>(), req);
			if (prevChainResponse) {
				return prevChainResponse;
			}
			return {this->matchAll(req)};
		}

		// Called within catch handler when any chain elements throw. Must not
		// throw.
		virtual void onRequestException() {
			Rain::Error::consumeThrowable([]() { throw; }, RAIN_ERROR_LOCATION)();
		}

		// R/R Worker overrides.

		// Handle non-error Request. Must not throw. Invokes chains in order.
		virtual bool onRequest(Request &req) final override {
			try {
				// Preprocessor chain already invoked on req.

				// Invoke match chain. The final element in the chain guarantees a
				// match.
				PreResponse preRes = this->chainMatchTarget(req).value();

				// If the PreResponse doesn't contain a Response, abort if toClose, or
				// just listen for next Request.
				if (!preRes) {
					if (preRes.toClose) {
						this->abort();
						return true;
					} else {
						return false;
					}
				}

				// PreRes contains a response. Send and possibly close. Postprocessor
				// chain invoked automatically.
				*this << preRes.value();

				if (
					preRes.toClose || preRes.value().version == Version::_0_9 ||
					preRes.value().version == Version::_1_0) {
					this->close();
					return true;
				}
				return false;
			} catch (...) {
				this->onRequestException();
				return true;
			}
		};

		// Catch exceptions during request receiving. Must not throw.
		virtual void onRecvRequestException() final override {
			try {
				throw;
			} catch (typename Request::Exception const &exception) {
				// Send back a helpful error code if possible.
				switch (exception.getError()) {
					case Request::Error::HTTP_VERSION_NOT_SUPPORTED:
						*this << Response(StatusCode::HTTP_VERSION_NOT_SUPPORTED);
						break;
					case Request::Error::METHOD_NOT_ALLOWED:
						*this << Response(StatusCode::METHOD_NOT_ALLOWED);
						break;
					case Request::Error::MALFORMED_VERSION:
					case Request::Error::MALFORMED_HEADERS:
					case Request::Error::MALFORMED_BODY:
						*this << Response(StatusCode::BAD_REQUEST);
						break;
					default:
						*this << Response(StatusCode::INTERNAL_SERVER_ERROR);
						break;
				}

				// Gracefully close so peer can see error message.
				this->close();
			}
		}

		// Continue pp chains.
		private:
		virtual void streamOutImpl(Tag<Interface>, Response &) {}
		virtual void streamInImpl(Tag<Interface>, Request &) {}

		virtual void streamOutImpl(Tag<SuperInterface>, Response &res)
			final override {
			this->streamOutImpl(Tag<Interface>(), res);

			// Default postprocessors include Content-Length and Content-Type.
			res.ppEstimateContentLength();
			res.ppDefaultContentType();
		}
		virtual void streamInImpl(Tag<SuperInterface>, Request &req)
			final override {
			// No preprocessors.
			this->streamInImpl(Tag<Interface>(), req);
		}

		public:
		using SuperInterface::operator<<;
		using SuperInterface::operator>>;
	};

	typedef WorkerInterface<Socket> Worker;
}

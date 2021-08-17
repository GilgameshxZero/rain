// HTTP Worker specialization.
#pragma once

#include "../req-res/worker.hpp"
#include "socket.hpp"

#include <optional>
#include <regex>
#include <unordered_set>

namespace Rain::Networking::Http {
	// HTTP Worker specialization.
	class WorkerSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public ReqRes::WorkerSocketSpecInterfaceInterface {
		protected:
		// For easy binding to request handlers.
		template <typename Handler, typename... Args>
		auto bind(Handler &&handler, Args &&...args) {
			return std::bind(
				std::forward<Handler>(handler),
				std::forward<Args>(args)...,
				std::placeholders::_1,
				std::placeholders::_2);
		}
	};

	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class WorkerSocketSpecInterface
			: virtual public WorkerSocketSpecInterfaceInterface,
				virtual public ReqRes::
					WorkerSocketSpecInterface<RequestMessageSpec, ResponseMessageSpec> {
		protected:
		// Requests are delegated to handlers which return a ResponseAction
		// consisting of an optional response and an optional flag. A
		// ResponseMessageSpec is sent if available, and in either case, if the flag
		// is set, the connection is closed afterwards. If no flag exists, then this
		// is treated as "no action", and the next action-getter should be
		// attempted.
		//
		// The state with a response and no flag is invalid and cannot be
		// constructed.
		class ResponseAction {
			public:
			std::optional<ResponseMessageSpec> response;

			// If true, closes after the ResponseMessageSpec is sent, or aborts with
			// no ResponseMessageSpec.
			std::optional<bool> toClose;

			// No action, proceed to next filter.
			ResponseAction() : toClose(), response() {}

			// Send response, optionally close.
			ResponseAction(ResponseMessageSpec &&response, bool toClose = false)
					: response(std::forward<ResponseMessageSpec>(response)),
						toClose(toClose) {}

			// Don't send response, optionally close.
			ResponseAction(std::nullptr_t, bool toClose)
					: response(), toClose(toClose) {}

			ResponseAction(ResponseAction const &) = delete;
			ResponseAction &operator=(ResponseAction const &) = delete;
			ResponseAction(ResponseAction &&other) = delete;
			ResponseAction &operator=(ResponseAction &&) = delete;
		};

		// Subclasses specify behavior by defining filters for incoming requests.
		class RequestFilter {
			public:
			std::regex host;
			std::regex target;
			std::set<Method> methods;

			// If the filter matches, handlers can choose to return a ResponseAction,
			// or not. If no ResponseAction is returned, the next filter is checked.
			//
			// void * is type erasure.
			std::function<
				ResponseAction(void *, RequestMessageSpec &, std::smatch const &)>
				handler;

			// Handlers are bound inside this function.
			template <typename Host, typename Target, typename Derived>
			RequestFilter(
				Host &&host,
				Target &&target,
				std::set<Method> &&methods,
				// Handler must be a member function of a derived type, or else
				// static_cast will crash.
				ResponseAction (
					Derived::*memFn)(RequestMessageSpec &, std::smatch const &))
					: host(std::forward<Host>(host)),
						target(std::forward<Target>(target)),
						methods(std::forward<std::set<Method>>(methods)),
						handler([memFn](
											void *instance,
											RequestMessageSpec &req,
											std::smatch const &match) {
							// This lambda essentially stores the type Derived.
							return (*static_cast<Derived *>(instance).*memFn)(req, match);
						}) {}
		};
	};

	template <
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename Socket>
	class WorkerSocketSpec : public Socket,
													 virtual public WorkerSocketSpecInterface<
														 RequestMessageSpec,
														 ResponseMessageSpec>,
													 virtual public WorkerSocketSpecInterfaceInterface {
		using Socket::Socket;

		protected:
		// Import dependent names. Must prefix with Http:: to specify the templated
		// dependent typename, otherwise compiler will interpret this as the Tcp::
		// non-templated non-dependent typename during phase 1 of name resolution.
		using typename Http::WorkerSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec>::ResponseAction;
		using typename Http::WorkerSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec>::RequestFilter;

		// Shorthands available for the subclass.
		using Request = RequestMessageSpec;
		using Response = ResponseMessageSpec;

		private:
		// Subclasses define behavior by overriding a virtual list of handlers. This
		// function will only be called at most once per instance and saved. Should
		// return at least one filter.
		virtual std::vector<RequestFilter> filters() = 0;
		std::vector<RequestFilter> _filters;

		// Called when filter handlers throw. Returns false to continue to receiving
		// next ResponseMessageSpec, and true to close the connection.
		virtual bool onFilterException() {
			Rain::Error::consumeThrowable(
				[this]() {
					this->send(ResponseMessageSpec{StatusCode::INTERNAL_SERVER_ERROR});
					this->shutdown();
					throw;
				},
				RAIN_ERROR_LOCATION)();
			return true;
		}

		// R/R Worker overrides.

		// Handle non-error Request. Must not throw.
		virtual bool onRequest(RequestMessageSpec &req) final override {
			// Run through filters and try to send.
			if (this->_filters.empty()) {
				this->_filters = this->filters();
			}
			std::smatch targetMatch;
			for (RequestFilter const &filter : this->_filters) {
				if (
					std::regex_match(req.headers.host().asStr(), filter.host) &&
					std::regex_match(req.target, targetMatch, filter.target) &&
					filter.methods.find(req.method) != filter.methods.end()) {
					// Matched, call handler and process.
					try {
						// This will be cast to a derived type.
						ResponseAction result = filter.handler(this, req, targetMatch);

						// Ignore the rest of the unprocessed body.
						req.body.ignore(std::numeric_limits<std::streamsize>::max());

						// Did the filter decide to make an action?
						if (result.toClose) {
							if (result.response) {
								// Version of the response will always be set equal to the
								// version of the request.
								result.response.value().version = req.version;
								try {
									this->send(result.response.value());
								} catch (...) {
									return true;
								}
							}

							// Always close if the version < 1.1.
							if (
								result.toClose.value() || req.version == Version::_0_9 ||
								req.version == Version::_1_0) {
								Rain::Error::consumeThrowable([this]() { this->shutdown(); })();
								return true;
							}
							return false;
						}

						// If result is empty, try next filter.
					} catch (...) {
						return this->onFilterException();
					}
				}
			}

			// No filters returned a ResponseAction, send 404.
			try {
				this->send(
					ResponseMessageSpec{StatusCode::NOT_FOUND, {}, {}, {}, req.version});
				if (req.version == Version::_0_9 || req.version == Version::_1_0) {
					Rain::Error::consumeThrowable([this]() { this->shutdown(); })();
					return true;
				}
				return false;
			} catch (...) {
				return true;
			}
		};

		// Catch exceptions during request receiving. Must not throw.
		virtual void onRequestException() final override {
			try {
				throw;
			} catch (
				typename RequestMessageSpecInterface::Exception const &exception) {
				// Send back a helpful error code if possible.
				switch (exception.getError()) {
					case RequestMessageSpecInterface::Error::HTTP_VERSION_NOT_SUPPORTED:
						this->send(
							ResponseMessageSpec{StatusCode::HTTP_VERSION_NOT_SUPPORTED});
						break;
					case RequestMessageSpecInterface::Error::METHOD_NOT_ALLOWED:
						this->send(ResponseMessageSpec{StatusCode::METHOD_NOT_ALLOWED});
						break;
					case RequestMessageSpecInterface::Error::MALFORMED_VERSION:
					case RequestMessageSpecInterface::Error::MALFORMED_HEADERS:
					case RequestMessageSpecInterface::Error::MALFORMED_BODY:
						this->send(ResponseMessageSpec{StatusCode::BAD_REQUEST});
						break;
					default:
						this->send(
							ResponseMessageSpec{StatusCode::INTERNAL_SERVER_ERROR, {}});
						break;
				}

				// Gracefully close so peer can see error message.
				Rain::Error::consumeThrowable([this]() { this->shutdown(); })();
			}

			// No exceptions should leak through here. Any exceptions leaking through
			// will get caught by consumeThrowable on ReqRes Worker.
		}
	};

	// Shorthand for HTTP Worker.
	template <
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		std::size_t sendBufferLen,
		std::size_t recvBufferLen,
		long long sendTimeoutMs,
		long long recvTimeoutMs,
		typename SocketFamilyInterface,
		typename SocketTypeInterface,
		typename SocketProtocolInterface,
		template <typename>
		class... SocketOptions>
	class Worker
			: public WorkerSocketSpec<
					RequestMessageSpec,
					ResponseMessageSpec,
					ConnectedSocketSpec<NamedSocketSpec<SocketSpec<ReqRes::Worker<
						RequestMessageSpec,
						ResponseMessageSpec,
						sendBufferLen,
						recvBufferLen,
						sendTimeoutMs,
						recvTimeoutMs,
						SocketFamilyInterface,
						SocketTypeInterface,
						SocketProtocolInterface,
						SocketOptions...>>>>> {
		using WorkerSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<NamedSocketSpec<SocketSpec<ReqRes::Worker<
				RequestMessageSpec,
				ResponseMessageSpec,
				sendBufferLen,
				recvBufferLen,
				sendTimeoutMs,
				recvTimeoutMs,
				SocketFamilyInterface,
				SocketTypeInterface,
				SocketProtocolInterface,
				SocketOptions...>>>>>::WorkerSocketSpec;
	};
}

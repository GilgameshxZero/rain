// SMTP Worker specialization.
#pragma once

#include "../../algorithm/kmp.hpp"
#include "../../string/base-64.hpp"
#include "../req-res/worker.hpp"
#include "auth-method.hpp"
#include "mailbox.hpp"
#include "socket.hpp"

#include <optional>
#include <unordered_set>

namespace Rain::Networking::Smtp {
	// SMTP Worker specialization.
	class WorkerSocketSpecInterfaceInterface
			: virtual public ConnectedSocketSpecInterface,
				virtual public ReqRes::WorkerSocketSpecInterfaceInterface {
		protected:
		// Custom streambuf which reads data after DATA.
		class DataIStreamBuf : public std::streambuf {
			private:
			// The TCP Socket stream from which we will read from to fill up this
			// streambuf.
			std::istream *const sourceStream;

			// Internal buffer.
			std::string buffer;

			// KMP match for terminator \r\n.\r\n candidate.
			std::size_t candidate;
			char *match;

			public:
			DataIStreamBuf(
				std::istream *sourceStream,
				std::size_t bufferLen = 1_zu << 10)
					: sourceStream(sourceStream), candidate(0), match(nullptr) {
				this->buffer.reserve(bufferLen);
				this->buffer.resize(this->buffer.capacity());

				// Set internal pointers for empty buffers.
				this->setg(&this->buffer[0], &this->buffer[0], &this->buffer[0]);
			}

			// Disable copy.
			DataIStreamBuf(DataIStreamBuf const &) = delete;
			DataIStreamBuf &operator=(DataIStreamBuf const &) = delete;

			protected:
			// Re-fill the buffer from the source until \r\n.\r\n.
			virtual int_type underflow() noexcept override {
				// Only refill buffer if it has been exhausted.
				if (this->gptr() == this->egptr()) {
					// If previously matched terminator, we are done.
					if (this->match != nullptr) {
						return traits_type::eof();
					}

					// Receive the next block up to delimiter \n.
					if (!this->sourceStream->good()) {
						return traits_type::eof();
					}
					this->sourceStream->get(&this->buffer[0], this->buffer.length());
					std::size_t cFilled =
						static_cast<std::size_t>(this->sourceStream->gcount());
					this->buffer[cFilled++] = '\n';
					this->sourceStream->get();

					// Run matcher. A match will set this->match to an invalid, but
					// non-null location.
					static std::string const terminator{"\r\n.\r\n"};
					static auto const terminatorPartialMatch =
						Algorithm::computeKmpPartialMatch(terminator);
					std::tie(this->match, this->candidate) = Algorithm::kmpSearch(
						&this->buffer[0],
						cFilled,
						terminator,
						terminatorPartialMatch,
						this->candidate);

					// Regardless of match results (interpreted next time), return current
					// bytes.
					this->setg(
						&this->buffer[0], &this->buffer[0], &this->buffer[0] + cFilled);
				}
				return traits_type::to_int_type(*this->gptr());
			}
		};
	};

	template <typename RequestMessageSpec, typename ResponseMessageSpec>
	class WorkerSocketSpecInterface
			: virtual public WorkerSocketSpecInterfaceInterface,
				virtual public ReqRes::
					WorkerSocketSpecInterface<RequestMessageSpec, ResponseMessageSpec> {
		protected:
		class ResponseAction {
			public:
			std::optional<ResponseMessageSpec> response;

			// If true, closes after the ResponseMessageSpec is sent, or aborts with
			// no ResponseMessageSpec.
			bool toClose;

			// Close.
			ResponseAction() : toClose(true), response() {}

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
		protected:
		// Import dependent names. Must prefix with Http:: to specify the templated
		// dependent typename, otherwise compiler will interpret this as the Tcp::
		// non-templated non-dependent typename during phase 1 of name resolution.
		using typename Smtp::WorkerSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec>::ResponseAction;

		// Shorthands available for the subclass.
		using Request = RequestMessageSpec;
		using Response = ResponseMessageSpec;

		// Internal state.
		std::optional<Mailbox> mailFrom;
		std::unordered_set<Mailbox, HashMailbox> rcptTo;

		// Protected to allow for overrides to call if necessary.

		// Delegate handlers for subclass overriding.
		virtual bool onInitialResponse() override {
			this->send(ResponseMessageSpec{StatusCode::SERVICE_READY});
			return false;
		}
		virtual ResponseAction onHelo(RequestMessageSpec &) {
			return {{StatusCode::REQUEST_COMPLETED}};
		}
		virtual ResponseAction onMailMailbox(Mailbox const &mailbox) {
			// By default, accept all MAIL FROM, replacing earlier commands with this
			// one.
			this->mailFrom = mailbox;
			return {{StatusCode::REQUEST_COMPLETED}};
		}
		virtual ResponseAction onMail(RequestMessageSpec &req) {
			try {
				// Parse the parameter between the <, >. Does not modify the original
				// parameter.
				std::stringstream parameterStream(req.parameter);
				parameterStream.ignore(
					std::numeric_limits<std::streamsize>::max(), '<');
				std::string mailboxStr;
				std::getline(parameterStream, mailboxStr, '>');

				// Delegate to another handler.
				return this->onMailMailbox({mailboxStr});
			} catch (...) {
				// Exceptions cause bad status code.
				return {{StatusCode::SYNTAX_ERROR_PARAMETER_ARGUMENT}};
			}
		}

		// onRcpt subhandler after parsing mailbox.
		virtual ResponseAction onRcptMailbox(Mailbox const &mailbox) {
			if (this->rcptTo.size() > (1_zu << 10)) {
				return {{StatusCode::REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE}};
			}
			this->rcptTo.insert(mailbox);
			return {{StatusCode::REQUEST_COMPLETED}};
		}

		virtual ResponseAction onRcpt(RequestMessageSpec &req) {
			try {
				// Parse the parameter between the <, >. Does not modify the original
				// parameter.
				std::stringstream parameterStream(req.parameter);
				parameterStream.ignore(
					std::numeric_limits<std::streamsize>::max(), '<');
				std::string mailboxStr;
				std::getline(parameterStream, mailboxStr, '>');

				// Delegate to another handler.
				return this->onRcptMailbox({mailboxStr});
			} catch (...) {
				// Exceptions cause bad status code.
				return {{StatusCode::SYNTAX_ERROR_PARAMETER_ARGUMENT}};
			}
		}

		// Subclasses override this to return a ResponseMessageSpec in response to a
		// data istream.
		virtual ResponseAction onDataStream(std::istream &stream) {
			// Ignore all the data, then reject it.
			stream.ignore(std::numeric_limits<std::streamsize>::max());
			return {{StatusCode::TRANSACTION_FAILED}};
		}

		virtual ResponseAction onData(RequestMessageSpec &) {
			// Accept data as long as at to/from addresses have been issued already.
			if (!this->mailFrom || this->rcptTo.size() == 0) {
				return {{StatusCode::BAD_SEQUENCE_COMMAND}};
			}

			// Send a 354, and set up the stream for streaming the data.
			this->send(ResponseMessageSpec{StatusCode::START_MAIL_INPUT});
			std::unique_ptr<std::streambuf> dataIStreamBuf(new DataIStreamBuf(this));
			std::istream stream(dataIStreamBuf.get());

			// Return no ResponseMessageSpec without closing.
			return this->onDataStream(stream);
		}

		virtual ResponseAction onRset(RequestMessageSpec &) {
			this->mailFrom.reset();
			this->rcptTo.clear();
			return {{StatusCode::REQUEST_COMPLETED, {{"OK"s}}}};
		}
		virtual ResponseAction onNoop(RequestMessageSpec &) {
			return {{StatusCode::REQUEST_COMPLETED, {{"OK"s}}}};
		}
		virtual ResponseAction onQuit(RequestMessageSpec &) {
			return {{StatusCode::SERVICE_CLOSING}, true};
		}
		virtual ResponseAction onSend(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_NOT_IMPLEMENTED}};
		}
		virtual ResponseAction onSoml(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_NOT_IMPLEMENTED}};
		}
		virtual ResponseAction onSaml(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_NOT_IMPLEMENTED}};
		}
		virtual ResponseAction onVrfy(RequestMessageSpec &) {
			return {{StatusCode::CANNOT_VERIFY}};
		}
		virtual ResponseAction onExpn(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_NOT_IMPLEMENTED}};
		}
		virtual ResponseAction onHelp(RequestMessageSpec &) {
			return {{StatusCode::HELP_MESSAGE}};
		}
		virtual ResponseAction onTurn(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_NOT_IMPLEMENTED}};
		}

		virtual ResponseAction onEhlo(RequestMessageSpec &req) {
			return this->onHelo(req);
		}

		// AUTH delegates to virtual method handlers after parsing.
		virtual ResponseAction onAuthPlain(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED}};
		}
		// Allow subclass override to provide authentication on username/password
		// from AUTH LOGIN.
		virtual ResponseAction onAuthLogin(
			std::string const &,
			std::string const &) {
			// Reject all by default.
			return {{StatusCode::AUTHENTICATION_INVALID}};
		}
		virtual ResponseAction onAuthLogin(RequestMessageSpec &) {
			// Default AUTH LOGIN handler parses for username and password and sends
			// to delegate handler.

			static std::string const usernamePromptB64 =
																 String::Base64::encode("Username"),
															 passwordPromptB64 =
																 String::Base64::encode("Password");

			// Client AUTH username/password can be at most ~1K.
			std::string usernameB64(1_zu << 10, '\0'), passwordB64(1_zu << 10, '\0');
			this->send(ResponseMessageSpec{
				StatusCode::SERVER_CHALLENGE, {{usernamePromptB64}}});
			this->getline(&usernameB64[0], usernameB64.length());
			// -2 for \r delimiter.
			usernameB64.resize(static_cast<std::size_t>(
				std::max(std::streamsize(0), this->gcount() - 2)));
			this->send(ResponseMessageSpec{
				StatusCode::SERVER_CHALLENGE, {{passwordPromptB64}}});
			this->getline(&passwordB64[0], passwordB64.length());
			passwordB64.resize(static_cast<std::size_t>(
				std::max(std::streamsize(0), this->gcount() - 2)));

			// Decode both from Base64 and delegate.
			std::string username = String::Base64::decode(usernameB64),
									password = String::Base64::decode(passwordB64);
			return this->onAuthLogin(username, password);
		}
		virtual ResponseAction onAuthCramMd5(RequestMessageSpec &) {
			return {{StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED}};
		}
		virtual ResponseAction onAuth(RequestMessageSpec &req) {
			AuthMethod authMethod;
			try {
				// First token of parameter is AuthMethod.
				authMethod =
					AuthMethod(req.parameter.substr(0, req.parameter.find(' ')));
			} catch (...) {
				return {{StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED}};
			}

			switch (authMethod) {
				case AuthMethod::PLAIN:
					return this->onAuthPlain(req);
				case AuthMethod::LOGIN:
					return this->onAuthLogin(req);
				case AuthMethod::CRAM_MD5:
					return this->onAuthCramMd5(req);
				default:
					return {{StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED}};
			}
		}

		private:
		// Subclass override.
		virtual bool onCommandException() {
			Rain::Error::consumeThrowable(
				[this]() {
					this->send(
						ResponseMessageSpec{StatusCode::REQUEST_ABORTED_LOCAL_ERROR});
					this->shutdown();
					throw;
				},
				RAIN_ERROR_LOCATION)();
			return true;
		}

		// R/R Worker overrides.
		//
		// Handle non-error RequestMessageSpec. Must not throw. Invokes chains in
		// order.
		virtual bool onRequest(RequestMessageSpec &req) final override {
			// Get PreResponse based on the command.
			auto result = [this, &req]() -> ResponseAction {
				try {
					switch (req.command) {
						case Command::HELO:
						// Default never occurs since req.command is enforced.
						default:
							return this->onHelo(req);
						case Command::MAIL:
							return this->onMail(req);
						case Command::RCPT:
							return this->onRcpt(req);
						case Command::DATA:
							return this->onData(req);
						case Command::RSET:
							return this->onRset(req);
						case Command::NOOP:
							return this->onNoop(req);
						case Command::QUIT:
							return this->onQuit(req);
						case Command::SEND:
							return this->onSend(req);
						case Command::SOML:
							return this->onSoml(req);
						case Command::SAML:
							return this->onSaml(req);
						case Command::VRFY:
							return this->onVrfy(req);
						case Command::EXPN:
							return this->onExpn(req);
						case Command::HELP:
							return this->onHelp(req);
						case Command::TURN:
							return this->onTurn(req);

						case Command::EHLO:
							return this->onEhlo(req);
						case Command::AUTH:
							return this->onAuth(req);
					}
				} catch (...) {
					// Any exceptions during processing are caught to the subclass
					// handler.
					return {nullptr, this->onCommandException()};
				}
			}();

			if (result.response) {
				try {
					this->send(result.response.value());
				} catch (...) {
					return true;
				}
			}

			if (result.toClose) {
				Rain::Error::consumeThrowable([this]() { this->shutdown(); })();
				return true;
			}
			return false;
		};

		// Catch exceptions during request receiving. Must not throw.
		virtual void onRequestException() final override {
			try {
				throw;
			} catch (typename RequestMessageSpec::Exception const &exception) {
				Rain::Error::consumeThrowable([this, exception]() {
					switch (exception.getError()) {
						case RequestMessageSpec::Error::SYNTAX_ERROR_COMMAND:
							this->send(ResponseMessageSpec{StatusCode::SYNTAX_ERROR_COMMAND});
							break;
						default:
							this->send(ResponseMessageSpec{StatusCode::TRANSACTION_FAILED});
							break;
					}

					// Graceful close so peer can see message.
					this->shutdown();
				})();
			}

			// No exceptions should leak through here. Any exceptions leaking through
			// will get caught by consumeThrowable on ReqRes Worker.
		}
	};

	// Shorthand for SMTP Worker.
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

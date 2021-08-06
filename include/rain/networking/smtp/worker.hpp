// SMTP Worker specialization.
#pragma once

#include "../../algorithm/kmp.hpp"
#include "../../string/base-64.hpp"
#include "../request-response/worker.hpp"
#include "auth-method.hpp"
#include "mailbox.hpp"
#include "socket.hpp"

#include <optional>
#include <unordered_set>

namespace Rain::Networking::Smtp {
	// SMTP Worker specialization.
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

		protected:
		// Internal state.
		std::optional<Mailbox> mailFrom;
		std::unordered_set<Mailbox, HashMailbox> rcptTo;

		// Subclass handlers return PreResponses which allow for no response, or
		// close after response.
		class PreResponse {
			private:
			std::optional<Response> optional;

			public:
			bool toClose;

			// Default constructor means that no response is returned and the
			// connection is aborted.
			PreResponse() : optional(), toClose(true) {}

			// A PreResponse with a Response will always be sent. The connection will
			// be gracefully closed afterwards if toClose is set.
			template <typename... MessageArgs>
			PreResponse(
				StatusCode statusCode,
				std::vector<std::string> &&lines = {},
				bool toClose = false,
				MessageArgs &&...args)
					: optional(
							std::in_place,
							statusCode,
							std::move(lines),
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

		private:
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
					std::streamsize cFilled = this->sourceStream->gcount();
					this->buffer[cFilled++] = '\n';
					this->sourceStream->get();

					// Run matcher. A match will set this->match to an invalid, but
					// non-null location.
					static std::string const terminator = "\r\n.\r\n";
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

		// Protected to allow for overrides to call if necessary.
		protected:
		// Delegate handlers for subclass overriding.
		virtual bool onInitialResponse() override {
			*this << Response(StatusCode::SERVICE_READY);
			return false;
		}
		virtual PreResponse onHelo(Request &) {
			return {StatusCode::REQUEST_COMPLETED};
		}
		virtual PreResponse onMail(Request &req) {
			try {
				// Parse the parameter between the <, >. Does not modify the original
				// parameter.
				std::string parameter(req.parameter.substr(5));
				String::trimWhitespace(parameter);
				this->mailFrom = Mailbox(parameter.substr(1, parameter.length() - 2));

				return {StatusCode::REQUEST_COMPLETED};
			} catch (...) {
				// Exceptions cause bad status code.
				return {StatusCode::SYNTAX_ERROR_PARAMETER_ARGUMENT};
			}
		}

		// onRcpt subhandler after parsing mailbox. Recommended to add mailbox to
		// this->rctpTo on success.
		virtual PreResponse onRcptMailbox(Mailbox const &mailbox) {
			// Default behavior makes no checks.
			this->rcptTo.insert(mailbox);
			return {StatusCode::REQUEST_COMPLETED};
		}

		virtual PreResponse onRcpt(Request &req) {
			try {
				// Parse the parameter between the <, >. Does not modify the original
				// parameter.
				std::string parameter(req.parameter.substr(3));
				String::trimWhitespace(parameter);

				// Delegate to another subclass handler, since RCPT should return 550 if
				// mailbox doesn't exist.
				return this->onRcptMailbox(
					Mailbox(parameter.substr(1, parameter.length() - 2)));
			} catch (...) {
				// Exceptions cause bad status code.
				return {StatusCode::SYNTAX_ERROR_PARAMETER_ARGUMENT};
			}
		}

		// Subclasses override this to return a Response in response to a data
		// istream.
		virtual PreResponse onDataStream(std::istream &stream) {
			// Ignore all the data, then reject it.
			stream.ignore(std::numeric_limits<std::streamsize>::max());
			return {StatusCode::TRANSACTION_FAILED};
		}

		virtual PreResponse onData(Request &) {
			// Accept data as long as at from/to addresses have been issued already.
			if (!this->mailFrom || this->rcptTo.size() == 0) {
				return {StatusCode::BAD_SEQUENCE_COMMAND};
			}

			// Send a 354, and set up the stream for streaming the data.
			*this << Response(StatusCode::START_MAIL_INPUT);

			std::unique_ptr<std::streambuf> dataIStreamBuf(new DataIStreamBuf(this));
			std::istream stream(dataIStreamBuf.get());

			// Return no Response without closing.
			return this->onDataStream(stream);
		}

		protected:
		virtual PreResponse onRset(Request &) {
			this->mailFrom.reset();
			this->rcptTo.clear();
			return {StatusCode::REQUEST_COMPLETED, {{"OK"s}}};
		}
		virtual PreResponse onNoop(Request &) {
			return {StatusCode::REQUEST_COMPLETED, {{"OK"s}}};
		}
		virtual PreResponse onQuit(Request &) {
			return {StatusCode::SERVICE_CLOSING, {}, true};
		}
		virtual PreResponse onSend(Request &) {
			return {StatusCode::COMMAND_NOT_IMPLEMENTED};
		}
		virtual PreResponse onSoml(Request &) {
			return {StatusCode::COMMAND_NOT_IMPLEMENTED};
		}
		virtual PreResponse onSaml(Request &) {
			return {StatusCode::COMMAND_NOT_IMPLEMENTED};
		}
		virtual PreResponse onVrfy(Request &) {
			return {StatusCode::CANNOT_VERIFY};
		}
		virtual PreResponse onExpn(Request &) {
			return {StatusCode::COMMAND_NOT_IMPLEMENTED};
		}
		virtual PreResponse onHelp(Request &) { return {StatusCode::HELP_MESSAGE}; }
		virtual PreResponse onTurn(Request &) {
			return {StatusCode::COMMAND_NOT_IMPLEMENTED};
		}

		virtual PreResponse onEhlo(Request &req) { return this->onHelo(req); }

		// AUTH delegates to virtual method handlers after parsing.
		virtual PreResponse onAuthPlain(Request &) {
			return {StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED};
		}
		// Allow subclass override to provide authentication on username/password
		// from AUTH LOGIN.
		virtual PreResponse onAuthLogin(std::string const &, std::string const &) {
			// Reject all by default.
			return {StatusCode::AUTHENTICATION_INVALID};
		}
		virtual PreResponse onAuthLogin(Request &) {
			// Default AUTH LOGIN handler parses for username and password and sends
			// to delegate handler.

			static std::string const usernamePromptB64 =
																 String::Base64::encode("Username"),
															 passwordPromptB64 =
																 String::Base64::encode("Password");

			// Client AUTH username/password can be at most ~1K.
			std::string usernameB64(1_zu << 10, '\0'), passwordB64(1_zu << 10, '\0');
			*this << Response(StatusCode::SERVER_CHALLENGE, {{usernamePromptB64}});
			this->getline(&usernameB64[0], usernameB64.length());
			// -2 for \r delimiter.
			usernameB64.resize(std::max(std::streamsize(0), this->gcount() - 2));
			*this << Response(StatusCode::SERVER_CHALLENGE, {{passwordPromptB64}});
			this->getline(&passwordB64[0], passwordB64.length());
			passwordB64.resize(std::max(std::streamsize(0), this->gcount() - 2));

			// Decode both from Base64 and delegate.
			std::string username = String::Base64::decode(usernameB64),
									password = String::Base64::decode(passwordB64);
			return this->onAuthLogin(username, password);
		}
		virtual PreResponse onAuthCramMd5(Request &) {
			return {StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED};
		}
		virtual PreResponse onAuth(Request &req) {
			AuthMethod authMethod;
			try {
				// First four characters of parameter is AuthMethod.
				authMethod = AuthMethod(req.parameter.substr(0, 4));
			} catch (...) {
				return {StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED};
			}

			switch (authMethod) {
				case AuthMethod::PLAIN:
					return this->onAuthPlain(req);
				case AuthMethod::LOGIN:
					return this->onAuthLogin(req);
				case AuthMethod::CRAM_MD5:
					return this->onAuthCramMd5(req);
				default:
					return {StatusCode::COMMAND_PARAMETER_NOT_IMPLEMENTED};
			}
		}

		private:
		// Subclass override.
		virtual void onRequestException() {
			Rain::Error::consumeThrowable([]() { throw; }, RAIN_ERROR_LOCATION)();
		}

		// R/R Worker overrides.
		//
		// Handle non-error Request. Must not throw. Invokes chains in order.
		virtual bool onRequest(Request &req) final override {
			// Get PreResponse based on the command.
			PreResponse preRes = [this, &req]() -> PreResponse {
				try {
					switch (req.command) {
						case Command::HELO:
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
						// Default never occurs since req.command is enforced.
						default:
							return this->onAuth(req);
					}
				} catch (...) {
					// Any exceptions during processing are caught to the subclass
					// handler.
					this->onRequestException();

					// Return empty PreResponse with toClose true to abort.
					return {};
				}
			}();

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

			if (preRes.toClose) {
				this->close();
				return true;
			}
			return false;
		};

		// Catch exceptions during request receiving. Must not throw.
		virtual void onRecvRequestException() final override {
			try {
				throw;
			} catch (typename Request::Exception const &exception) {
				switch (exception.getError()) {
					case Request::Error::SYNTAX_ERROR_COMMAND:
						*this << Response(StatusCode::SYNTAX_ERROR_COMMAND);
						break;
					default:
						*this << Response(StatusCode::TRANSACTION_FAILED);
						break;
				}

				// Graceful close so peer can see message.
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
			// No postprocessors.
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

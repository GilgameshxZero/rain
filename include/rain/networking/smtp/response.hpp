// Response-specific SMTP parsing.
#pragma once

#include "../request-response/response.hpp"
#include "message.hpp"
#include "status-code.hpp"

#include <vector>

namespace Rain::Networking::Smtp {
	template <typename ProtocolMessage>
	class ResponseInterface
			: public RequestResponse::ResponseInterface<ProtocolMessage> {
		public:
		typedef ProtocolMessage Message;

		private:
		// SuperInterface aliases the superclass.
		typedef RequestResponse::ResponseInterface<Message> SuperInterface;

		// Interface aliases this class.
		typedef ResponseInterface<Message> Interface;

		protected:
		// Tag aliases for sendWith/recvWith convenience.
		typedef typename SuperInterface::InterfaceTag SuperTag;
		typedef typename SuperInterface::template Tag<Interface> InterfaceTag;

		public:
		// Exception class carries over most errors from Message parsing.
		enum class Error { INVALID_STATUS_CODE = 1, LINES_LIMIT_EXCEEDED };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Networking::Smtp::Response";
			}
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::INVALID_STATUS_CODE:
						return "Invalid status code.";
					case Error::LINES_LIMIT_EXCEEDED:
						return "Too many lines in response.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		// Three-digit status code from RFC 821.
		StatusCode statusCode;

		// Lines in the response.
		std::vector<std::string> lines;

		// Carry over constructor which recvs from a Socket.
		template <typename... MessageArgs>
		ResponseInterface(
			StatusCode statusCode = StatusCode::REQUEST_COMPLETED,
			// No lines will simply take the reason phrase from statusCode on a single
			// line.
			std::vector<std::string> &&lines = {},
			MessageArgs &&...args)
				: SuperInterface(
						// bind version to rvalue reference to be perfect forwarded by
						// SuperInterface to Message.
						std::forward<MessageArgs>(args)...),
					statusCode(statusCode),
					lines(std::move(lines)) {}
		ResponseInterface(ResponseInterface &&other)
				: SuperInterface(std::move(other)),
					statusCode(other.statusCode),
					lines(std::move(other.lines)) {}

		// Not copyable.
		ResponseInterface(ResponseInterface const &) = delete;
		ResponseInterface &operator=(ResponseInterface const &) = delete;

		private:
		// Provided for subclass override.
		virtual void sendWithImpl(InterfaceTag, std::ostream &){};
		virtual void recvWithImpl(InterfaceTag, std::istream &){};

		// Overrides for Super versions implement protocol behavior.
		virtual void sendWithImpl(SuperTag, std::ostream &stream) final override {
			this->sendWithImpl(InterfaceTag(), stream);

			// If no lines, append the default reason phrase.
			if (this->lines.empty()) {
				this->lines.push_back(this->statusCode.getReasonPhrase());
			}

			// For every line but the final, send CODE-line.
			for (std::size_t index = 0; index + 1 < this->lines.size(); index++) {
				stream << this->statusCode << "-" << this->lines[index] << "\r\n";
			}
			stream << this->statusCode << " " << this->lines.back() << "\r\n";
			stream.flush();
		}
		virtual void recvWithImpl(SuperTag, std::istream &stream) final override {
			this->recvWithImpl(InterfaceTag(), stream);

			// Receive line-by-line, up to 4K bytes total.
			std::size_t totalBytes = 0;
			while (totalBytes < (1_zu << 12)) {
				try {
					// statusCode is always the first three characters.
					std::string statusCodeStr(3, '\0');
					stream.read(&statusCodeStr[0], statusCodeStr.length());
					this->statusCode = StatusCode(statusCodeStr);
				} catch (...) {
					throw Exception(Error::INVALID_STATUS_CODE);
				}

				// The next character determines whether it is the final line.
				char statusDelimiter = static_cast<char>(stream.get());

				// Each line can be at most ~1K characters.
				this->lines.emplace_back(1_zu << 10, '\0');
				stream.getline(&this->lines.back()[0], this->lines.back().length());
				// -2 to discard the \r\0.
				this->lines.back().resize(
					std::max(std::streamsize(0), stream.gcount() - 2));
				totalBytes += this->lines.back().length();

				// Exit if last line.
				if (statusDelimiter == ' ') {
					return;
				}
			}

			// If here, the response is too large.
			throw Exception(Error::LINES_LIMIT_EXCEEDED);
		}

		private:
	};

	// Response final specialization.
	typedef ResponseInterface<Message> Response;
}

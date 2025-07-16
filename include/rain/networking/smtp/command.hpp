// SMTP request command.
#pragma once

#ifdef DELETE
#undef DELETE
#endif

#include "../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Smtp {
	// SMTP request command (HELO, MAIL, RCPT, DATA, QUIT, etc.).
	class Command {
		public:
		// Unscoped enum allows for direct name resolution from wrapper class.
		enum Value {
			HELO = 0,
			MAIL,
			RCPT,
			DATA,
			RSET,
			NOOP,
			QUIT,
			SEND,
			SOML,
			SAML,
			VRFY,
			EXPN,
			HELP,
			TURN,

			EHLO,
			AUTH
		};

		private:
		// Keep private to enforce value as one of the possible enum values.
		Value value;

		public:
		// Maps strings to Value. Commands are case-agnostic.
		static inline std::unordered_map<
			std::string,
			Value,
			String::CaseAgnosticHash,
			String::CaseAgnosticEqual> const fromStr{
			{"HELO", HELO},
			{"MAIL", MAIL},
			{"RCPT", RCPT},
			{"DATA", DATA},
			{"RSET", RSET},
			{"NOOP", NOOP},
			{"QUIT", QUIT},
			{"SEND", SEND},
			{"SOML", SOML},
			{"SAML", SAML},
			{"VRFY", VRFY},
			{"EXPN", EXPN},
			{"HELP", HELP},
			{"TURN", TURN},

			{"EHLO", EHLO},
			{"AUTH", AUTH}};

		// Direct constructors. Parsing strings may throw.
		constexpr Command(Value value = HELO) noexcept : value(value) {}
		Command(std::string const &str) : value(Command::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case HELO:
					return "HELO";
				case MAIL:
					return "MAIL";
				case RCPT:
					return "RCPT";
				case DATA:
					return "DATA";
				case RSET:
					return "RSET";
				case NOOP:
					return "NOOP";
				case QUIT:
					return "QUIT";
				case SEND:
					return "SEND";
				case SOML:
					return "SOML";
				case SAML:
					return "SAML";
				case VRFY:
					return "VRFY";
				case EXPN:
					return "EXPN";
				case HELP:
					return "HELP";
				case TURN:
					return "TURN";
				case EHLO:
					return "EHLO";
				case AUTH:
				// Default never occurs since value is private.
				default:
					return "AUTH";
			}
		}
	};
}

// Stream operators.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Smtp::Command command) {
	return stream << static_cast<std::string>(command);
}
inline std::istream &operator>>(
	std::istream &stream,
	Rain::Networking::Smtp::Command &command) {
	// Any stream-in operations should not be subject to failure from a long line.
	//
	// Reserve +2 characters, one for the terminating nullptr, one for reading the
	// delimiter. failbit will not be set if the maximum length is reached.
	std::string commandStr(6, '\0');
	stream.getline(&commandStr[0], commandStr.size(), ' ');
	commandStr.resize(static_cast<std::size_t>(
		std::max(std::streamsize(0), stream.gcount() - 1)));
	command = Rain::Networking::Smtp::Command(commandStr);
	return stream;
}

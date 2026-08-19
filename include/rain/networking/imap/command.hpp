// IMAP message command (e.g. NOOP, OK).
#pragma once

#include "../../string/string.hpp"

#include <iostream>
#include <unordered_map>

namespace Rain::Networking::Imap {
	// IMAP message command.
	class Command {
		public:
		enum Value {
			NOOP = 0,
			LOGIN,
			LOGOUT,
			SELECT,
			CHECK,
			EXPUNGE,
			COPY,
			FETCH,
			STORE,
			SEARCH,

			OK,
			NO,
			BAD,
			FLAGS,
			BYE,

			// Empty command string which resolves to "".
			NONE
		};

		private:
		Value value;

		public:
		// Maps strings to Value. Commands are case-agnostic.
		static inline std::unordered_map<
			std::string,
			Value,
			String::CaseAgnosticHash,
			String::CaseAgnosticEqual> const fromStr{
			{"NOOP", NOOP},
			{"LOGIN", LOGIN},
			{"LOGOUT", LOGOUT},
			{"SELECT", SELECT},
			{"CHECK", CHECK},
			{"EXPUNGE", EXPUNGE},
			{"COPY", COPY},
			{"FETCH", FETCH},
			{"STORE", STORE},
			{"SEARCH", SEARCH},

			{"OK", OK},
			{"NO", NO},
			{"BAD", BAD},
			{"FLAGS", FLAGS},
			{"BYE", BYE},

			{"", NONE}};

		// Direct constructors. Parsing strings may throw.
		constexpr Command(Value value = NONE) noexcept :
			value(value) {}
		Command(std::string const &str) :
			value(Command::fromStr.at(str)) {}

		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case NOOP:
					return "NOOP";
				case LOGIN:
					return "LOGIN";
				case LOGOUT:
					return "LOGOUT";
				case SELECT:
					return "SELECT";
				case CHECK:
					return "CHECK";
				case EXPUNGE:
					return "EXPUNGE";
				case COPY:
					return "COPY";
				case FETCH:
					return "FETCH";
				case STORE:
					return "STORE";
				case SEARCH:
					return "SEARCH";

				case OK:
					return "OK";
				case NO:
					return "NO";
				case BAD:
					return "BAD";
				case FLAGS:
					return "FLAGS";
				case BYE:
					return "BYE";

				case NONE:
				default:
					return "";
			}
		}

		// Stream operators.
		friend inline std::ostream &operator<<(
			std::ostream &stream,
			Rain::Networking::Imap::Command command) {
			return stream << static_cast<std::string>(command);
		}
		friend inline std::istream &operator>>(
			std::istream &stream,
			Rain::Networking::Imap::Command &command) {
			// Any stream-in operations should not be subject to
			// failure from a long line.
			//
			// Buffer size is important here. `failbit` will not
			// be set if the maximum length is reached.
			std::string commandStr(16, '\0');
			stream.getline(
				&commandStr[0], commandStr.size(), ' ');
			commandStr.resize(
				static_cast<std::size_t>(std::max(
					std::streamsize(0), stream.gcount() - 1)));
			command = Rain::Networking::Imap::Command(commandStr);
			return stream;
		}
	};
}

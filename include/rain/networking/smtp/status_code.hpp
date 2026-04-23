// SMTP response status code, and optional unwrapping into
// reason phrase.
#pragma once

#include "../../string/string.hpp"

#include <iostream>

namespace Rain::Networking::Smtp {
	// SMTP response status code, and optional unwrapping into
	// reason phrase.
	class StatusCode {
		public:
		// Categorization for status codes.
		enum class Category {
			POSITIVE_PRELIMINARY = 1,
			POSITIVE_CONFIRMATION,
			POSITIVE_INTERMEDIATE,
			TRANSIENT_NEGATIVE,
			PERMANENT_NEGATIVE
		};

		// Unscoped enum allows for direct name resolution from
		// wrapper class.
		enum Value {
			SYSTEM_STATUS = 211,
			HELP_MESSAGE = 214,
			SERVICE_READY = 220,
			SERVICE_CLOSING = 221,
			AUTHENTICATION_SUCCEEDED = 235,
			REQUEST_COMPLETED = 250,
			USER_NOT_LOCAL = 251,
			CANNOT_VERIFY = 252,

			SERVER_CHALLENGE = 334,
			START_MAIL_INPUT = 354,

			SERVICE_NOT_AVAILABLE = 421,
			PASSWORD_TRANSITION_NEEDED = 432,
			REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE = 450,
			REQUEST_ABORTED_LOCAL_ERROR = 451,
			REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE = 452,
			TEMPORARY_AUTHENTICATION_FAILURE = 454,
			CANNOT_ACCOMODATE_PARAMETER = 455,

			SYNTAX_ERROR_COMMAND = 500,
			SYNTAX_ERROR_PARAMETER_ARGUMENT = 501,
			COMMAND_NOT_IMPLEMENTED = 502,
			BAD_SEQUENCE_COMMAND = 503,
			COMMAND_PARAMETER_NOT_IMPLEMENTED = 504,
			SERVER_NO_MAIL = 521,
			ENCRYPTION_NEEDED = 523,
			AUTHENTICATION_REQUIRED = 530,
			AUTHENTICATION_TOO_WEAK = 534,
			AUTHENTICATION_INVALID = 535,
			AUTHENTICATION_REQUIRES_ENCRYPTION = 538,
			REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT = 550,
			USER_NOT_LOCAL_PERMANENT = 551,
			REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT = 552,
			REQUEST_NOT_TAKEN_MAILBOX_NAME = 553,
			TRANSACTION_FAILED = 554,
			DOMAIN_NO_MAIL = 556,
		};

		private:
		// Keep private to enforce value as one of the possible
		// enum values.
		Value value;

		public:
		// Direct constructors. Directly casts to Value from
		// std::size_t or parses std::string. May throw.
		constexpr StatusCode(
			Value value = REQUEST_COMPLETED) noexcept
				: value(value) {}
		StatusCode(std::size_t value)
				: value(static_cast<Value>(value)) {}
		StatusCode(std::string const &value)
				: StatusCode(
						static_cast<std::size_t>(
							std::strtoumax(value.c_str(), NULL, 10))) {}

		// Enable switch via getValue and getCategory.
		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const {
			return std::to_string(this->value);
		}

		// Special getters.
		Category getCategory() const noexcept {
			switch (this->value) {
				case SYSTEM_STATUS:
				case HELP_MESSAGE:
				case SERVICE_READY:
				case SERVICE_CLOSING:
				case AUTHENTICATION_SUCCEEDED:
				case REQUEST_COMPLETED:
				case USER_NOT_LOCAL:
				case CANNOT_VERIFY:
					return Category::POSITIVE_CONFIRMATION;

				case SERVER_CHALLENGE:
				case START_MAIL_INPUT:
					return Category::POSITIVE_INTERMEDIATE;

				case SERVICE_NOT_AVAILABLE:
				case PASSWORD_TRANSITION_NEEDED:
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE:
				case REQUEST_ABORTED_LOCAL_ERROR:
				case REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE:
				case TEMPORARY_AUTHENTICATION_FAILURE:
				case CANNOT_ACCOMODATE_PARAMETER:
					return Category::TRANSIENT_NEGATIVE;

				case SYNTAX_ERROR_COMMAND:
				case SYNTAX_ERROR_PARAMETER_ARGUMENT:
				case COMMAND_NOT_IMPLEMENTED:
				case BAD_SEQUENCE_COMMAND:
				case COMMAND_PARAMETER_NOT_IMPLEMENTED:
				case SERVER_NO_MAIL:
				case ENCRYPTION_NEEDED:
				case AUTHENTICATION_REQUIRED:
				case AUTHENTICATION_TOO_WEAK:
				case AUTHENTICATION_INVALID:
				case AUTHENTICATION_REQUIRES_ENCRYPTION:
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT:
				case USER_NOT_LOCAL_PERMANENT:
				case REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT:
				case REQUEST_NOT_TAKEN_MAILBOX_NAME:
				case TRANSACTION_FAILED:
				case DOMAIN_NO_MAIL:
				// Default never occurs.
				default:
					return Category::PERMANENT_NEGATIVE;
			}
		}
		std::string getReasonPhrase() const noexcept {
			switch (this->value) {
				case SYSTEM_STATUS:
					return "Default system status";
				case HELP_MESSAGE:
					return "Default help message";
				case SERVICE_READY:
					return "Service ready";
				case SERVICE_CLOSING:
					return "Service closing transmission channel";
				case AUTHENTICATION_SUCCEEDED:
					return "Authentication succeeded";
				case REQUEST_COMPLETED:
					return "Requested mail action okay, completed";
				case USER_NOT_LOCAL:
					return "User not local; will forward";
				case CANNOT_VERIFY:
					return "Cannot verify user";

				case SERVER_CHALLENGE:
					return "Default server challenge";
				case START_MAIL_INPUT:
					return "Start mail input; end with <CRLF>.<CRLF>";

				case PASSWORD_TRANSITION_NEEDED:
					return "A password transition is needed";
				case SERVICE_NOT_AVAILABLE:
					return "Service not available, closing "
								 "transmission channel";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE:
					return "Requested mail action not taken: mailbox "
								 "unavailable "
								 "(temporary)";
				case REQUEST_ABORTED_LOCAL_ERROR:
					return "Requested action aborted: local error in "
								 "processing "
								 "(temporary)";
				case REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE:
					return "Requested action not taken: insufficient "
								 "system storage "
								 "(temporary)";
				case TEMPORARY_AUTHENTICATION_FAILURE:
					return "Temporary authentication failure";
				case CANNOT_ACCOMODATE_PARAMETER:
					return "Server unable to accommodate parameters "
								 "(temporary)";

				case SYNTAX_ERROR_COMMAND:
					return "Syntax error, command unrecognized";
				case SYNTAX_ERROR_PARAMETER_ARGUMENT:
					return "Syntax error in parameters or arguments";
				case COMMAND_NOT_IMPLEMENTED:
					return "Command not implemented";
				case BAD_SEQUENCE_COMMAND:
					return "Bad sequence of commands";
				case COMMAND_PARAMETER_NOT_IMPLEMENTED:
					return "Command parameter not implemented";
				case SERVER_NO_MAIL:
					return "Server does not accept mail";
				case ENCRYPTION_NEEDED:
					return "Encryption Needed";
				case AUTHENTICATION_REQUIRED:
					return "Authentication required";
				case AUTHENTICATION_TOO_WEAK:
					return "Authentication mechanism is too weak";
				case AUTHENTICATION_INVALID:
					return "Authentication credentials invalid";
				case AUTHENTICATION_REQUIRES_ENCRYPTION:
					return "Encryption required for requested "
								 "authentication mechanism";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT:
					return "Requested action not taken: mailbox "
								 "unavailable (permanent)";
				case USER_NOT_LOCAL_PERMANENT:
					return "User not local";
				case REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT:
					return "Requested mail action aborted: exceeded "
								 "storage allocation "
								 "(permanent)";
				case REQUEST_NOT_TAKEN_MAILBOX_NAME:
					return "Requested action not taken: mailbox name "
								 "not allowed "
								 "(permanent)";
				case TRANSACTION_FAILED:
					return "Transaction failed (permanent)";
				case DOMAIN_NO_MAIL:
				// Default never occurs.
				default:
					return "Domain does not accept mail";
			}
		}

		// Stream operator.
		friend inline std::ostream &operator<<(
			std::ostream &stream,
			Rain::Networking::Smtp::StatusCode statusCode) {
			return stream << static_cast<std::string>(statusCode);
		}
	};
}

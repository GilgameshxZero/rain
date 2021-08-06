// SMTP response status code, and optional unwrapping into reason phrase.
#pragma once

#include "../../string/string.hpp"

#include <iostream>

namespace Rain::Networking::Smtp {
	// SMTP response status code, and optional unwrapping into reason phrase.
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

		// Unscoped enum allows for direct name resolution from wrapper class.
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
		// Keep private to enforce value as one of the possible enum values.
		Value value;

		public:
		// Maps std::size_t to Value.
		inline static std::unordered_map<std::size_t, Value> const fromSz{
			{211, SYSTEM_STATUS},
			{214, HELP_MESSAGE},
			{220, SERVICE_READY},
			{221, SERVICE_CLOSING},
			{235, AUTHENTICATION_SUCCEEDED},
			{250, REQUEST_COMPLETED},
			{251, USER_NOT_LOCAL},
			{252, CANNOT_VERIFY},

			{334, SERVER_CHALLENGE},
			{354, START_MAIL_INPUT},

			{421, SERVICE_NOT_AVAILABLE},
			{432, PASSWORD_TRANSITION_NEEDED},
			{450, REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE},
			{451, REQUEST_ABORTED_LOCAL_ERROR},
			{452, REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE},
			{454, TEMPORARY_AUTHENTICATION_FAILURE},
			{455, CANNOT_ACCOMODATE_PARAMETER},

			{500, SYNTAX_ERROR_COMMAND},
			{501, SYNTAX_ERROR_PARAMETER_ARGUMENT},
			{502, COMMAND_NOT_IMPLEMENTED},
			{503, BAD_SEQUENCE_COMMAND},
			{504, COMMAND_PARAMETER_NOT_IMPLEMENTED},
			{521, SERVER_NO_MAIL},
			{523, ENCRYPTION_NEEDED},
			{530, AUTHENTICATION_REQUIRED},
			{534, AUTHENTICATION_TOO_WEAK},
			{535, AUTHENTICATION_INVALID},
			{538, AUTHENTICATION_REQUIRES_ENCRYPTION},
			{550, REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT},
			{551, USER_NOT_LOCAL_PERMANENT},
			{552, REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT},
			{553, REQUEST_NOT_TAKEN_MAILBOX_NAME},
			{554, TRANSACTION_FAILED},
			{556, DOMAIN_NO_MAIL}};

		// Direct constructors. Directly casts to Value from std::size_t or parses
		// std::string. May throw.
		constexpr StatusCode(Value value = REQUEST_COMPLETED) noexcept
				: value(value) {}
		StatusCode(std::size_t value) : value(StatusCode::fromSz.at(value)) {}
		StatusCode(std::string const &value)
				: StatusCode(std::strtoumax(value.c_str(), NULL, 10)) {}

		// Enable switch via getValue and getCategory.
		// Conversions and comparators.
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			switch (this->value) {
				case SYSTEM_STATUS:
					return "211";
				case HELP_MESSAGE:
					return "214";
				case SERVICE_READY:
					return "220";
				case SERVICE_CLOSING:
					return "221";
				case AUTHENTICATION_SUCCEEDED:
					return "235";
				case REQUEST_COMPLETED:
					return "250";
				case USER_NOT_LOCAL:
					return "251";
				case CANNOT_VERIFY:
					return "252";

				case SERVER_CHALLENGE:
					return "334";
				case START_MAIL_INPUT:
					return "354";

				case SERVICE_NOT_AVAILABLE:
					return "421";
				case PASSWORD_TRANSITION_NEEDED:
					return "432";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE:
					return "450";
				case REQUEST_ABORTED_LOCAL_ERROR:
					return "451";
				case REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE:
					return "452";
				case TEMPORARY_AUTHENTICATION_FAILURE:
					return "454";
				case CANNOT_ACCOMODATE_PARAMETER:
					return "455";

				case SYNTAX_ERROR_COMMAND:
					return "500";
				case SYNTAX_ERROR_PARAMETER_ARGUMENT:
					return "501";
				case COMMAND_NOT_IMPLEMENTED:
					return "502";
				case BAD_SEQUENCE_COMMAND:
					return "503";
				case COMMAND_PARAMETER_NOT_IMPLEMENTED:
					return "504";
				case SERVER_NO_MAIL:
					return "521";
				case ENCRYPTION_NEEDED:
					return "523";
				case AUTHENTICATION_REQUIRED:
					return "530";
				case AUTHENTICATION_TOO_WEAK:
					return "534";
				case AUTHENTICATION_INVALID:
					return "535";
				case AUTHENTICATION_REQUIRES_ENCRYPTION:
					return "538";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT:
					return "550";
				case USER_NOT_LOCAL_PERMANENT:
					return "551";
				case REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT:
					return "552";
				case REQUEST_NOT_TAKEN_MAILBOX_NAME:
					return "553";
				case TRANSACTION_FAILED:
					return "554";
				// Default never occurs.
				case DOMAIN_NO_MAIL:
				default:
					return "556";
			}
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
					return "Service not available, closing transmission channel";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE:
					return "Requested mail action not taken: mailbox unavailable "
								 "(temporary)";
				case REQUEST_ABORTED_LOCAL_ERROR:
					return "Requested action aborted: local error in processing "
								 "(temporary)";
				case REQUEST_NOT_TAKEN_INSUFFICIENT_STORAGE:
					return "Requested action not taken: insufficient system storage "
								 "(temporary)";
				case TEMPORARY_AUTHENTICATION_FAILURE:
					return "Temporary authentication failure";
				case CANNOT_ACCOMODATE_PARAMETER:
					return "Server unable to accommodate parameters (temporary)";

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
					return "Encryption required for requested authentication mechanism";
				case REQUEST_NOT_TAKEN_MAILBOX_UNAVAILABLE_PERMANENT:
					return "Requested action not taken: mailbox unavailable (permanent)";
				case USER_NOT_LOCAL_PERMANENT:
					return "User not local";
				case REQUEST_ABORTED_INSUFFICIENT_STORAGE_PERMANENT:
					return "Requested mail action aborted: exceeded storage allocation "
								 "(permanent)";
				case REQUEST_NOT_TAKEN_MAILBOX_NAME:
					return "Requested action not taken: mailbox name not allowed "
								 "(permanent)";
				case TRANSACTION_FAILED:
					return "Transaction failed (permanent)";
				case DOMAIN_NO_MAIL:
				// Default never occurs.
				default:
					return "Domain does not accept mail";
			}
		}
	};
}

// Stream operator.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Smtp::StatusCode statusCode) {
	return stream << static_cast<std::string>(statusCode);
}

// HTTP status code, and optional unwrapping into reason
// phrase.
#pragma once

#include "../../string/string.hpp"

#include <iostream>

namespace Rain::Networking::Http {
	// HTTP status code, and optional unwrapping into reason
	// phrase.
	class StatusCode {
		public:
		// Categorization for status codes.
		enum class Category {
			INFORMATIONAL = 1,
			SUCCESS,
			REDIRECTION,
			CLIENT_ERROR,
			SERVER_ERROR
		};

		// Unscoped enum allows for direct name resolution from
		// wrapper class.
		enum Value {
			CONTINUE = 100,
			SWITCHING_PROTOCOLS = 101,

			OK = 200,
			CREATED = 201,
			ACCEPTED = 202,
			NON_AUTHORITATIVE_INFORMATION = 203,
			NO_CONTENT = 204,
			RESET_CONTENT = 205,
			PARTIAL_CONTENT = 206,

			MULTIPLE_CHOICES = 300,
			MOVED_PERMANENTLY = 301,
			FOUND = 302,
			SEE_OTHER = 303,
			NOT_MODIFIED = 304,
			USE_PROXY = 305,
			TEMPORARY_REDIRECT = 306,

			BAD_REQUEST = 400,
			UNAUTHORIZED = 401,
			PAYMENT_REQUIRED = 402,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
			METHOD_NOT_ALLOWED = 405,
			NOT_ACCEPTABLE = 406,
			PROXY_AUTHENTICATION_REQUIRED = 407,
			REQUEST_TIMEOUT = 408,
			CONFLICT = 409,
			GONE = 410,
			LENGTH_REQUIRED = 411,
			PRECONDITION_FAILED = 412,
			REQUEST_ENTITY_TOO_LARGE = 413,
			REQUEST_URI_TOO_LARGE = 414,
			UNSUPPORTED_MEDIA_TYPE = 415,
			REQUESTED_RANGE_NOT_SATISFIABLE = 416,
			EXPECTATION_FAILED = 417,

			INTERNAL_SERVER_ERROR = 500,
			NOT_IMPLEMENTED = 501,
			BAD_GATEWAY = 502,
			SERVICE_UNAVAILABLE = 503,
			GATEWAY_TIMEOUT = 504,
			HTTP_VERSION_NOT_SUPPORTED = 505
		};

		private:
		// Keep private to enforce value as one of the possible
		// enum values.
		Value value;

		public:
		// Direct constructors. Directly casts to Value from
		// std::size_t or parses std::string. May throw.
		constexpr StatusCode(Value value = OK) noexcept
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
				case CONTINUE:
				case SWITCHING_PROTOCOLS:
					return Category::INFORMATIONAL;

				case OK:
				case CREATED:
				case ACCEPTED:
				case NON_AUTHORITATIVE_INFORMATION:
				case NO_CONTENT:
				case RESET_CONTENT:
				case PARTIAL_CONTENT:
					return Category::SUCCESS;

				case MULTIPLE_CHOICES:
				case MOVED_PERMANENTLY:
				case FOUND:
				case SEE_OTHER:
				case NOT_MODIFIED:
				case USE_PROXY:
				case TEMPORARY_REDIRECT:
					return Category::REDIRECTION;

				case BAD_REQUEST:
				case UNAUTHORIZED:
				case PAYMENT_REQUIRED:
				case FORBIDDEN:
				case NOT_FOUND:
				case METHOD_NOT_ALLOWED:
				case NOT_ACCEPTABLE:
				case PROXY_AUTHENTICATION_REQUIRED:
				case REQUEST_TIMEOUT:
				case CONFLICT:
				case GONE:
				case LENGTH_REQUIRED:
				case PRECONDITION_FAILED:
				case REQUEST_ENTITY_TOO_LARGE:
				case REQUEST_URI_TOO_LARGE:
				case UNSUPPORTED_MEDIA_TYPE:
				case REQUESTED_RANGE_NOT_SATISFIABLE:
				case EXPECTATION_FAILED:
					return Category::CLIENT_ERROR;

				case INTERNAL_SERVER_ERROR:
				case NOT_IMPLEMENTED:
				case BAD_GATEWAY:
				case SERVICE_UNAVAILABLE:
				case GATEWAY_TIMEOUT:
				case HTTP_VERSION_NOT_SUPPORTED:
				// Default never occurs.
				default:
					return Category::SERVER_ERROR;
			}
		}
		std::string getReasonPhrase() const noexcept {
			switch (this->value) {
				case CONTINUE:
					return "Continue";
				case SWITCHING_PROTOCOLS:
					return "Switching Protocols";

				case OK:
					return "OK";
				case CREATED:
					return "Created";
				case ACCEPTED:
					return "Accepted";
				case NON_AUTHORITATIVE_INFORMATION:
					return "Non-Authoritative Information";
				case NO_CONTENT:
					return "No Content";
				case RESET_CONTENT:
					return "Reset Content";
				case PARTIAL_CONTENT:
					return "Partial Content";

				case MULTIPLE_CHOICES:
					return "Multiple Choices";
				case MOVED_PERMANENTLY:
					return "Moved Permanently";
				case FOUND:
					return "Found";
				case SEE_OTHER:
					return "See Other";
				case NOT_MODIFIED:
					return "Not Modified";
				case USE_PROXY:
					return "Use Proxy";
				case TEMPORARY_REDIRECT:
					return "Temporary Redirect";

				case BAD_REQUEST:
					return "Bad Request";
				case UNAUTHORIZED:
					return "Unauthorized";
				case PAYMENT_REQUIRED:
					return "Payment Required";
				case FORBIDDEN:
					return "Forbidden";
				case NOT_FOUND:
					return "Not Found";
				case METHOD_NOT_ALLOWED:
					return "Method Not Allowed";
				case NOT_ACCEPTABLE:
					return "Not Acceptable";
				case PROXY_AUTHENTICATION_REQUIRED:
					return "Proxy Authentication Required";
				case REQUEST_TIMEOUT:
					return "Request Time-out";
				case CONFLICT:
					return "Conflict";
				case GONE:
					return "Gone";
				case LENGTH_REQUIRED:
					return "Length Required";
				case PRECONDITION_FAILED:
					return "Precondition Failed";
				case REQUEST_ENTITY_TOO_LARGE:
					return "Request Entity Too Large";
				case REQUEST_URI_TOO_LARGE:
					return "Request-URI Too Large";
				case UNSUPPORTED_MEDIA_TYPE:
					return "Unsupported Media Type";
				case REQUESTED_RANGE_NOT_SATISFIABLE:
					return "Requested Range Not Satisfiable";
				case EXPECTATION_FAILED:
					return "Expectation Failed";

				case INTERNAL_SERVER_ERROR:
					return "Internal Server Error";
				case NOT_IMPLEMENTED:
					return "Not Implemented";
				case BAD_GATEWAY:
					return "Bad Gateway";
				case SERVICE_UNAVAILABLE:
					return "Service Unavailable";
				case GATEWAY_TIMEOUT:
					return "Gateway Time-out";
				case HTTP_VERSION_NOT_SUPPORTED:
				default:
					return "HTTP Version Not Supported";
			}
		}

		// Stream operator.
		friend inline std::ostream &operator<<(
			std::ostream &stream,
			Rain::Networking::Http::StatusCode statusCode) {
			return stream << static_cast<std::string>(statusCode);
		}
	};
}

// A bundle of parsers, one for each key. When parsing, pass a key + a string
// value to parse; the previously set parser will parse the string value into
// the parsing store set earlier.
#pragma once

#include "../error/exception.hpp"
#include "../platform.hpp"
#include "../string/string.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace Rain::String {
	// A bundle of parsers, one defined for each key passed in. Parsing a
	// key-value pair will attempt to use the pre-defined parser for that key to
	// parse the value. Like any unchecked string parser, ill-formed strings will
	// cause undefined behavior.
	class KeyedParser {
		public:
		enum class Error : int { NO_PARSER_FOR_KEY = 1 };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept { return "Rain::String::KeyedParser"; }
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::NO_PARSER_FOR_KEY:
						return "Key does not have a parser.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		// Parser signature. The argument stores the string to be parsed. The
		// function implicitly knows where to store the result.
		typedef std::function<bool(std::string const &)> Parser;

		private:
		std::unordered_map<std::string, Parser> parsers;

		public:
		// Add a key parser by specifying the store location (and its type).
		// Adding a previously added key will fail and return true.
		template <typename Store>
		bool addParser(std::string const &key, Store &store) {
			return !this->parsers.insert(std::make_pair(key, this->getParser(store)))
								.second;
		}

		// Parse a text given its key. May throw exceptions. Returns true on parser
		// error.
		bool parse(std::string const &key, std::string const &value) const {
			auto const &it = this->parsers.find(key);
			if (it == this->parsers.end()) {
				throw Exception(Error::NO_PARSER_FOR_KEY);
			}
			return it->second(value);
		}

		private:
		// Overloads for getParser which return a value parser based on the type to
		// store into. Bools only parse to true if the string is some variation of
		// "true" or a non-zero number.
		//
		// Internally, rain uses the strto* variants instead of the sto* variants as
		// the former returns valid results on error and do not throw.
		Parser getParser(bool &store) const {
			return Parser([&store](std::string const &value) {
				if (std::strtoll(value.c_str(), nullptr, 10) != 0) {
					store = true;
				} else {
					std::string mValue(value);
					toLower(trimWhitespace(mValue));
					store = (value == "true");
				}
				return false;
			});
		}
		// long long parser.
		Parser getParser(long long &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::strtoll(value.c_str(), nullptr, 10);
				return false;
			});
		}
		// unsigned long long parser.
		Parser getParser(unsigned long long &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::strtoull(value.c_str(), nullptr, 10);
				return false;
			});
		}
		// long parser.
		Parser getParser(long &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::strtol(value.c_str(), nullptr, 10);
				return false;
			});
		}
		// unsigned long parser.
		Parser getParser(unsigned long &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::strtoul(value.c_str(), nullptr, 10);
				return false;
			});
		}
		// double parser.
		Parser getParser(double &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::stod(value);
				return false;
			});
		}
		// long double parser.
		Parser getParser(long double &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = std::stod(value);
				return false;
			});
		}
		// std::string parser.
		Parser getParser(std::string &store) const noexcept {
			return Parser([&store](std::string const &value) {
				store = value;
				return false;
			});
		}
		// std::vector parser.
		template <typename Inner>
		Parser getParser(std::vector<Inner> &store) const noexcept {
			return Parser([this, &store](std::string const &value) {
				store.push_back(Inner());
				this->getParser(store.back())(value);
				return false;
			});
		}
		// std::size_t parser not provided without std::enable_if. Use one of the
		// existing numerical types instead.
	};
}

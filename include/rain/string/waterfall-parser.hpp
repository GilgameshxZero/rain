#pragma once

#include "../error-exception/exception.hpp"
#include "../platform.hpp"
#include "../string.hpp"	// For cStrCmp.

#include <functional>
#include <map>
#include <vector>

namespace Rain::String {
	class WaterfallParser {
		public:
		// Custom `error_condition`s.
		enum class Error { NO_PARSER_FOR_KEY_TYPE = 0 };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::String::WaterfallParser";
			}
			std::string message(int ev) const noexcept {
				switch (static_cast<Error>(ev)) {
					case Error::NO_PARSER_FOR_KEY_TYPE:
						return "Key type does not have a parser.";
					default:
						return "Generic.";
				}
			}
		};

		// Compound type to store all information with a parser.
		struct Layer {
			// Parsing function.
			typedef std::function<void(char const *, void *)> Parser;

			// Constructor for inline initialization.
			Layer(void *const param, Parser const &parser)
					: param(param), parser(parser) {}

			void *const param;
			Parser const &parser;
		};

		// Attempt to parse a key with previously added parsers.
		// Can throw exceptions.
		void parse(char const *const key, char const *const sValue) const {
			auto it = this->layers.find(key);
			if (it == this->layers.end()) {
				throw ErrorException::Exception::makeException<ErrorCategory>(
					Error::NO_PARSER_FOR_KEY_TYPE);
			}
			it->second.parser(sValue, it->second.param);
		}

		// General: add a way to parse a key. Contains shared error-checking code
		// and builds the KeyHandler. Used as a shorthand by other addParser.
		template <typename StoreType>
		void addLayer(char const *const key, StoreType *store) noexcept {
			// Adding a key previously added will overwrite it.
			this->layers.insert(std::make_pair(
				key, Layer(reinterpret_cast<void *>(store), this->getParser(store))));
		}

		private:
		// Set by addLayer.
		std::map<char const *, Layer, cStrCmp> layers;

		// Return a parser based on the type passed in.
		Layer::Parser const &getParser(bool *) const noexcept {
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<bool *>(param) = !(std::strcmp(sValue, "0") == 0 ||
						std::strcmp(sValue, "false") == 0 ||
						std::strcmp(sValue, "False") == 0 ||
						std::strcmp(sValue, "FALSE") == 0);
				});
			return parser;
		}
		Layer::Parser const &getParser(long long *) const noexcept {
			static Layer::Parser const parser([](char const *const sValue,
																					void *const param) {
				*reinterpret_cast<long long *>(param) = std::strtoll(sValue, NULL, 10);
			});
			return parser;
		}
		Layer::Parser const &getParser(unsigned long long *) const noexcept {
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<unsigned long long *>(param) =
						std::strtoull(sValue, NULL, 10);
				});
			return parser;
		}
// For some reason, this function generates the same signature as `Layer::Parser
// const &getParser(unsigned long long *)` on GCC and Clang but not on MSVC, so
// we define it only on Windows.
#ifdef RAIN_WINDOWS
		Layer::Parser const &getParser(std::size_t *) const noexcept {
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<std::size_t *>(param) =
						static_cast<std::size_t>(std::strtoull(sValue, NULL, 10));
				});
			return parser;
		}
#endif
		Layer::Parser const &getParser(unsigned long *) const noexcept {
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<unsigned long *>(param) =
						std::strtoul(sValue, NULL, 10);
				});
			return parser;
		}
		Layer::Parser const &getParser(std::string *) const noexcept {
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					reinterpret_cast<std::string *>(param)->assign(sValue);
				});
			return parser;
		}
		template <typename TypeName>
		Layer::Parser const &getParser(
			std::vector<TypeName> *store) const noexcept {
			// Send some random pointer just to get the correct parser.
			static Layer::Parser const &typeParser =
				this->getParser(reinterpret_cast<TypeName *>(NULL));
			static Layer::Parser const parser(
				[](char const *const sValue, void *const param) {
					std::vector<TypeName> *store =
						reinterpret_cast<std::vector<TypeName> *>(param);
					store->push_back(TypeName());
					typeParser(sValue, reinterpret_cast<void *>(&store->back()));
				});
			return parser;
		}
	};
}

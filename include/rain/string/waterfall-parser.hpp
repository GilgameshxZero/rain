#pragma once

// For cStrCmp.
#include "../string.hpp"

#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

namespace Rain::String {
	class WaterfallParser {
		public:
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
				throw std::invalid_argument("Key does not have a parser.");
			}
			it->second.parser(sValue, it->second.param);
		}

		// General: add a way to parse a key. Contains shared error-checking code
		// and builds the KeyHandler. Used as a shorthand by other addParser.
		template <typename StoreType>
		void addLayer(char const *const key, StoreType *const store) noexcept {
			// Adding a key previously added will overwrite it.
			this->layers.insert(std::make_pair(
				key, Layer(reinterpret_cast<void *>(store), this->getParser(store))));
		}

		private:
		// Set by addLayer.
		std::map<char const *, Layer, cStrCmp> layers;

		// Return a parser based on the type passed in.
		Layer::Parser const &getParser(bool *) const noexcept {
			static const Layer::Parser parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<bool *>(param) = !(std::strcmp(sValue, "0") == 0 ||
						std::strcmp(sValue, "false") == 0 ||
						std::strcmp(sValue, "False") == 0 ||
						std::strcmp(sValue, "FALSE") == 0);
				});
			return parser;
		}
		Layer::Parser const &getParser(long long *) const noexcept {
			static const Layer::Parser parser([](char const *const sValue,
																					void *const param) {
				*reinterpret_cast<long long *>(param) = std::strtoll(sValue, NULL, 10);
			});
			return parser;
		}
		Layer::Parser const &getParser(unsigned long long *) const noexcept {
			static const Layer::Parser parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<unsigned long long *>(param) =
						std::strtoull(sValue, NULL, 10);
				});
			return parser;
		}
		Layer::Parser const &getParser(unsigned long *) const noexcept {
			static const Layer::Parser parser(
				[](char const *const sValue, void *const param) {
					*reinterpret_cast<unsigned long *>(param) =
						std::strtoul(sValue, NULL, 10);
				});
			return parser;
		}
		Layer::Parser const &getParser(std::string *) const noexcept {
			static const Layer::Parser parser(
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

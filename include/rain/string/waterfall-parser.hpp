#pragma once

// For cStrCmp.
#include "../string.hpp"

#include <functional>
#include <map>
#include <system_error>
#include <vector>

namespace Rain::String {
	class WaterfallParser {
		public:
		// Compound type to store all information with a parser.
		struct Layer {
			// Parsing function.
			typedef std::function<void(const char *, void *)> Parser;

			// Constructor for inline initialization.
			Layer(void *param, const Parser &parser) : param(param), parser(parser) {}

			void *param;
			const Parser parser;
		};

		// Attempt to parse a key with previously added parsers.
		// Can throw exceptions.
		void parse(const char *key, const char *sValue) const {
			auto it = this->layers.find(key);
			if (it == this->layers.end()) {
				throw std::system_error(1, std::generic_category());
			}
			it->second.parser(sValue, it->second.param);
		}

		// General: add a way to parse a key. Contains shared error-checking code
		// and builds the KeyHandler. Used as a shorthand by other addParser.
		template <typename StoreType>
		void addLayer(const char *key, StoreType *store) noexcept {
			// Adding a key previously added will overwrite it.
			this->layers.insert(std::make_pair(key,
				Layer(reinterpret_cast<void *>(store), this->getParser(store))));
		}

		private:
		// Set by addLayer.
		std::map<const char *, Layer, cStrCmp> layers;

		// Return a parser based on the type passed in.
		const Layer::Parser &getParser(bool *) const noexcept {
			static const Layer::Parser parser([](const char *sValue, void *param) {
				*reinterpret_cast<bool *>(param) = !(std::strcmp(sValue, "0") == 0 ||
					std::strcmp(sValue, "false") == 0 ||
					std::strcmp(sValue, "False") == 0 ||
					std::strcmp(sValue, "FALSE") == 0);
			});
			return parser;
		}
		const Layer::Parser &getParser(long long *) const noexcept {
			static const Layer::Parser parser([](const char *sValue, void *param) {
				*reinterpret_cast<long long *>(param) = std::strtoll(sValue, NULL, 10);
			});
			return parser;
		}
		const Layer::Parser &getParser(unsigned long long *) const noexcept {
			static const Layer::Parser parser([](const char *sValue, void *param) {
				*reinterpret_cast<unsigned long long *>(param) =
					std::strtoull(sValue, NULL, 10);
			});
			return parser;
		}
		const Layer::Parser &getParser(unsigned long *) const noexcept {
			static const Layer::Parser parser([](const char *sValue, void *param) {
				*reinterpret_cast<unsigned long *>(param) =
					std::strtoul(sValue, NULL, 10);
			});
			return parser;
		}
		const Layer::Parser &getParser(std::string *) const noexcept {
			static const Layer::Parser parser([](const char *sValue, void *param) {
				reinterpret_cast<std::string *>(param)->assign(sValue);
			});
			return parser;
		}
		template <typename TypeName>
		const Layer::Parser &getParser(std::vector<TypeName> *store) const
			noexcept {
			// Send some random pointer just to get the correct parser.
			static const Layer::Parser &typeParser =
				this->getParser(reinterpret_cast<TypeName *>(NULL));
			static const Layer::Parser parser([](const char *sValue, void *param) {
				std::vector<TypeName> *store =
					reinterpret_cast<std::vector<TypeName> *>(param);
				store->push_back(TypeName());
				typeParser(sValue, reinterpret_cast<void *>(&store->back()));
			});
			return parser;
		}
	};
}

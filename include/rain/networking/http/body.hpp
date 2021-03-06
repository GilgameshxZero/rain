// Abstract interface for loading bodies of payloads.
#pragma once

#include "../../platform.hpp"
#include "../../string.hpp"
#include "../request-response/socket.hpp"

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>

namespace Rain::Networking::Http {
	// Forward declarations.
	class Request;
	class Response;
	
	// TODO: Is this functional formulation optimal?
	class Body {
		public:
		// Generates some body bytes.
		typedef std::function<std::size_t(char **)> Generator;

		// Add stuff to body.
		// Adding a generator that gives 0 length will terminate the body.
		void appendGenerator(const Generator &generator) {
			std::lock_guard<std::mutex> generatorsLckGuard(this->generatorsMtx);
			this->generators.push(generator);
		}
		void appendBytes(const char *cStr, std::size_t len = 0) {
			if (len == 0) {
				len = std::strlen(cStr);
			}
			this->bytesLength += len;
			this->appendGenerator([cStr, len](char **bytes) {
				// Need const_cast here?
				*bytes = const_cast<char *>(cStr);
				return len;
			});
		}
		void appendBytes(const std::string &s) {
			this->appendBytes(s.c_str(), s.length());
		}

		// Extract stuff from body. Returns the number of bytes extracted.
		std::size_t extractBytes(char **bytes) {
			Generator generator;
			{
				std::lock_guard<std::mutex> generatorsLckGuard(this->generatorsMtx);
				if (this->generators.empty()) {	 // If none remaining, returns 0.
					return 0;
				}
				generator = this->generators.front();
				this->generators.pop();
			}
			return generator(bytes);
		}

		// Getter.
		std::size_t getBytesLength() { return this->bytesLength; }

		// Used with Request/Response subclasses.
		bool sendWith(RequestResponse::Socket<Request, Response> &socket) {
			char *bytes;
			std::size_t bytesLen = this->extractBytes(&bytes);
			while (bytesLen != 0) {
				socket.send(bytes, bytesLen);
				bytesLen = this->extractBytes(&bytes);
			}
			return false;
		}

		private:
		std::queue<Generator> generators;
		std::mutex generatorsMtx;

		// Length of body added through appendBytes.
		std::atomic_size_t bytesLength = 0;
	};
}

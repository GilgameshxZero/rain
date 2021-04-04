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

	class Body {
		public:
		// Bodies are composed of a series of generators, which modify a pointer to point to a buffer which contains the next piece of the body, returning the size of the buffer. The buffer is const char, and cannot be modified.
		typedef std::function<std::size_t(char const **)> Generator;

		private:
		std::queue<Generator> generators;
		std::mutex generatorsMtx;

		// Length of body added through appendBytes, only available if body is
		// "static".
		bool isStatic;
		std::atomic_size_t length;

		public:
		Body() : isStatic(true), length(0) {}

		// Do not append a generator which writes 0 bytes.
		void appendGenerator(Generator const &generator) noexcept {
			std::lock_guard<std::mutex> generatorsLckGuard(this->generatorsMtx);
			this->generators.push(generator);
			this->isStatic = false;
		}

		// Alternatively, append bytes directly to the body. The body does not store
		// the bytes.
		void appendBytes(char const *cStr, std::size_t len = 0) noexcept {
			if (len == 0) {
				len = std::strlen(cStr);
			}
			this->length += len;

			std::lock_guard<std::mutex> generatorsLckGuard(this->generatorsMtx);
			this->generators.push([cStr, len](char const **bytes) {
				// Need const_cast here?
				*bytes = cStr;
				return len;
			});
		}
		void appendBytes(std::string const &s) noexcept {
			this->appendBytes(s.c_str(), s.length());
		}

		// A body which has only accepted bytes and no general generators (a
		// "static" body) can be queried for its length.
		std::size_t getLength() const {
			if (this->isStatic) {
				return this->length;
			} else {
				throw std::runtime_error("Cannot get length of non-static Body.");
			}
		}
		bool getIsStatic() const noexcept { return this->isStatic; }

		// Extract stuff from body. Returns the number of bytes extracted.
		std::size_t extractBytes(char const **bytes) {
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

		// Used with Request/Response subclasses.
		bool sendWith(RequestResponse::Socket<Request, Response> &socket) {
			char const *bytes;
			std::size_t bytesLen = this->extractBytes(&bytes);
			while (bytesLen != 0) {
				if (socket.send(bytes, bytesLen)) {
					return true;
				}
				bytesLen = this->extractBytes(&bytes);
			}
			return false;
		}
	};
}

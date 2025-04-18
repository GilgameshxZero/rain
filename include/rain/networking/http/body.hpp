// HTTP body.
#pragma once

#include "../../literal.hpp"
#include "../../string/string.hpp"

#include <fstream>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <utility>

namespace Rain::Networking::Http {
	// HTTP body. A custom streambuf wrapped in an ostream.
	class Body : public std::istream {
		private:
		// Must be freed at the conclusion of the automatic Body.
		std::streambuf *toDelete;

		public:
		~Body() {
			if (this->toDelete != nullptr) {
				delete this->toDelete;
			}
		}

		// Build with static string.
		// Default constructor is empty body.
		Body(std::string const &str = "")
				: std::istream(str.empty() ? nullptr : new std::stringbuf(str)),
					toDelete(this->rdbuf()) {}
		Body(char const *cStr) : Body(std::string(cStr)) {}
		Body(std::stringbuf &&stringbuf)
				: std::istream(new std::stringbuf(std::move(stringbuf))),
					toDelete(this->rdbuf()) {}
		Body(std::filebuf &&filebuf)
				: std::istream(new std::filebuf(std::move(filebuf))),
					toDelete(this->rdbuf()) {}

		// Disable copy constructors.
		Body(Body const &) = delete;
		Body &operator=(Body const &) = delete;

		// Move constructor also swaps streambufs and freeing responsibility.
		Body(Body &&other) noexcept
				: std::istream(std::move(other)),
					toDelete(std::exchange(other.toDelete, nullptr)) {
			this->rdbuf(other.rdbuf(nullptr));
		}
		// Move assignment transfers memory responsibility to this object.
		Body &operator=(Body &&other) noexcept {
			std::istream::operator=(std::move(other));
			if (this->toDelete != nullptr) {
				delete this->toDelete;
			}
			this->toDelete = std::exchange(other.toDelete, nullptr);
			this->rdbuf(other.rdbuf(nullptr));
			return *this;
		}

		// Build with pre-existing streambuf. It must exist past the lifetime of
		// this Body.
		Body(std::streambuf *streamBuf)
				: std::istream(streamBuf), toDelete(nullptr) {}

		// Hide memory-sensitive setter.
		private:
		std::streambuf *rdbuf(std::streambuf *sb) {
			return std::istream::rdbuf(sb);
		}

		public:
		std::streambuf *rdbuf() const { return std::istream::rdbuf(); }

		// Helper for calling in_avail on rdbuf.
		std::streamsize inAvail() const {
			if (this->rdbuf() == nullptr) {
				return -1;
			}
			return this->rdbuf()->in_avail();
		}
	};
}

// Stream operator. Cannot stream in >> directly, since need to know
// Content-Length/Transfer-Encoding.
inline std::ostream &operator<<(
	std::ostream &stream,
	Rain::Networking::Http::Body &body) {
	// Will set failbit on stream if rdbuf is empty. So, only execute if rdbuf is
	// not empty.
	stream << body.rdbuf();
	stream.clear();
	return stream;
}

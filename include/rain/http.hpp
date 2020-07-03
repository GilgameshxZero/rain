#pragma once

#include "./socket.hpp"

#include <list>

namespace Rain {
	class HttpSocket : public Socket {
		public:
		// Receiving buffer.
		size_t bufSz;
		std::list<char *> bufs;

		HttpSocket(Socket socket = Socket(), size_t bufSz = 8192)
				: Socket(socket), bufSz(bufSz) {
			this->bufs.push_back(new char[bufSz]);
		}
		~HttpSocket() {
			// Free all buffers allocated in linked list.
			for (auto it = this->bufs.begin(); it != this->bufs.end(); it++) {
				delete *it;
			}
		}

		// Receive into buffer until double CRLF.
		int recvHeader(int timeout = 0) {
			auto it = this->bufs.begin();
			char *loc = *it, *end;

			int status, total = 0;
			while (true) {
				status =
					this->recv(reinterpret_cast<void *>(loc), this->bufSz - (loc - *it));
				if (status == 0) {
					// Graceful disconnect.
					break;
				} else if (status < 0) {
					// Error.
					return status;
				}

				// Stop if we encounter a double CRLF.
				end = reinterpret_cast<char *>(memmem(loc, status, "\r\n\r\n", 4));
				if (end != NULL) {
					return total + static_cast<int>(end - loc);
				}

				// Some bytes received.
				loc += status;
				total += status;

				// If we have used all of our buffer, allocate more.
				if (static_cast<size_t>(loc - *it) == this->bufSz &&
					++it == this->bufs.end()) {
					this->bufs.push_back(new char[this->bufSz]);
					loc = *(--it);
				}
			}

			return 0;
		}
	};
}

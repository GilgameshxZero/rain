#pragma once

#include "../platform/.hpp"

#include <list>
#include <mutex>
#include <stack>

namespace Rain::Memory {
	// Thread-safe efficient allocation of fixed-size buffers across the process.
	class BufferPool {
		public:
		BufferPool(size_t bufSz = 16384) : bufSz(bufSz) {}

		// Free all blocks whether they've been free by a user of BufferPool or not.
		~BufferPool() {
			for (auto it = this->blocks.begin(); it != this->blocks.end(); it++) {
				delete *it;
			}
		}

		// If we have previously used and freed a block, just give that back.
		char *getBuf() {
			this->freeBlocksMtx.lock();
			if (this->freeBlocks.size() == 0) {
				this->freeBlocksMtx.unlock();
				char *block = new char[this->bufSz];
				this->blocks.push_back(block);
				return block;
			}

			char *block = this->freeBlocks.top();
			this->freeBlocks.pop();
			this->freeBlocksMtx.unlock();
			return block;
		}

		// Mark the block as free again.
		void freeBuf(char *buf) {
			this->freeBlocksMtx.lock();
			this->freeBlocks.push(buf);
			this->freeBlocksMtx.unlock();
		}

		// Stats.
		size_t getBufSz() const { return this->bufSz; };
		size_t getCBlocks() const { return this->blocks.size(); }
		size_t getCFreeBlocks() const { return this->freeBlocks.size(); }
		double getUtil() const {
			if (this->blocks.size() == 0) {
				return 1;
			}
			return 1 -
				static_cast<double>(this->freeBlocks.size()) /
				static_cast<double>(this->blocks.size());
		}

		// Trim memory usage to achieve at least target utilization.
		void trim(double targetUtil) {
			size_t targetMaxFreeBlocks =
				static_cast<size_t>((1 - targetUtil) * this->blocks.size());
			this->freeBlocksMtx.lock();
			while (this->freeBlocks.size() > targetMaxFreeBlocks) {
				delete this->freeBlocks.top();
				this->freeBlocks.pop();
			}
			this->freeBlocksMtx.unlock();
		}

		private:
		size_t bufSz;

		// Thread-safe.
		std::mutex freeBlocksMtx;

		// All allocated blocks.
		std::list<char *> blocks;

		// All free blocks on the main block chain.
		std::stack<char *> freeBlocks;
	};
}

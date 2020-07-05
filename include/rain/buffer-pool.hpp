#pragma once

#include "./string.hpp"

#include <list>
#include <mutex>
#include <stack>

namespace Rain {
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

		size_t getBufSz() const { return this->bufSz; };

		// If we have previously used and freed a block, just give that back.
		char *newBuf() {
			this->freeBlocksMtx.lock();
			if (this->freeBlocks.size() == 0) {
				this->freeBlocksMtx.unlock();
				char *block = new char[this->bufSz + 1];
				block[0] = 1;
				this->blocks.push_back(block);
				return block + 1;
			}

			char *block = this->freeBlocks.top();
			this->freeBlocks.pop();
			this->freeBlocksMtx.unlock();
			block[0] = 1;
			return block + 1;
		}

		// Mark the block as free again.
		void deleteBuf(char *buf) {
			char *block = buf - 1;
			block[0] = 0;
			this->freeBlocksMtx.lock();
			this->freeBlocks.push(block);
			this->freeBlocksMtx.unlock();
		}

		// Stats.
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
		void freeToUtil(double targetUtil) {
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

		// Statically allocated blocks. Each block consists of 1 char of metadata
		// and the rest as buffer space. Metadata is 0 if the block is free.
		std::list<char *> blocks;

		// All free blocks on the main block chain.
		std::stack<char *> freeBlocks;
	};

	// Global static BufferPool.
	inline BufferPool &defaultBufferPool() {
		static BufferPool bufPool(8192);
		return bufPool;
	}
}

// Lossless variable-size Huffman encoding/decoding of texts consisting of at
// least 2 unique characters.
#pragma once

#include "../error/exception.hpp"
#include "../literal.hpp"

#include <array>
#include <climits>
#include <iostream>
#include <queue>
#include <vector>

namespace Rain::Algorithm {
	// Encode into or decode from an underlying stream.
	//
	// Final byte always stores the number of filler bits [0, 7] in the
	// second-to-last byte.
	class HuffmanStreamBuf : public std::streambuf {
		public:
		enum class Error {
			MALFORMED_DICTIONARY_TOO_MANY_NODES = 1,
			MALFORMED_DICTIONARY_LEAF_SEEN_TWICE,
			MALFORMED_DICTIONARY_ONE_DISTINCT,
			UNDERLYING_SBUMPC_FAILED,
			UNDERLYING_SPUTC_FAILED,
			CHARACTER_NOT_IN_DICTIONARY,
			CANNOT_PUT_AFTER_FLUSH
		};
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept { return "Rain::Algorithm::Huffman"; }
			std::string message(int error) const noexcept {
				switch (static_cast<Error>(error)) {
					case Error::MALFORMED_DICTIONARY_TOO_MANY_NODES:
						return "Too many non-leaf nodes created whlie constructing "
									 "dictionary from input bits.";
					case Error::MALFORMED_DICTIONARY_LEAF_SEEN_TWICE:
						return "The same leaf byte was encountered at least twice while "
									 "constructing dictionary from input bits.";
					case Error::UNDERLYING_SBUMPC_FAILED:
						return "Failed to bump character from underlying stream.";
					case Error::UNDERLYING_SPUTC_FAILED:
						return "Failed to put character to underlying stream.";
					case Error::CHARACTER_NOT_IN_DICTIONARY:
						return "While encoding, character in text was not found in "
									 "dictionary.";
					case Error::CANNOT_PUT_AFTER_FLUSH:
						return "Cannot put to underlying stream after sync/flush.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

		private:
		std::streambuf &underlying;

		// Do not carry an internal buffer, except for the bits which do not
		// constitute a full byte. Both buffers hold encrypted bits.
		std::pair<unsigned char, unsigned char> iBuffer{0, 0}, oBuffer{0, 0};

		// For overriding underflow, we need a get buffer of at least one char.
		char getArea;

		// To identify filler bits, we must read two characters ahead.
		std::pair<int_type, int_type> nextSBumpCs;

		// Tracks whether or not sync has been called/stream has been flushed. This
		// class cannot be used after flushing.
		bool flushed{false};

		// Represents Huffman dictionary in tree form & table form.
		//
		// The tree consists of up to 256 leaves and 255 non-leaf nodes, for up to
		// 511 nodes. The root and leaf count are stored separately, and each node
		// stores the indices of its children, with -1 denoting no child, and
		// indices under 256 denoting each of the 256 character leaf nodes.
		//
		// Observe that nodes have either 2 children or none, and the max-height of
		// the tree is 256 (with root height 0).
		//
		// The table maps each character to pair (M, B) where the first M bits of B
		// are the code for that character.
		//
		// Dictionaries may be encoded/decoded to/from bytestreams. The bytestream
		// representation of a tree is its pre-order traversal, with a single bit 0
		// representing a non-leaf node and 9 bits <1, C> representing a leaf node
		// for character C. This variable-length dictionary bytestream occupies up
		// to 511 + 256 * 8 = 2559 bits < 320 bytes.

		// Dictionaries contain [3, 511] nodes = leaves * 2 - 1. Node indices are
		// represented by shorts, with -1 representing NULL. Each pair represents
		// the indices of the two children.
		static inline short const MAX_NODES = UCHAR_MAX * 2 + 1;

		short root, parent[MAX_NODES];

		// Up to UCHAR_MAX = 255 non-leaf nodes may have children. The first 256
		// entries of this array are always {-1, -1}.
		std::pair<short, short> children[MAX_NODES];

		// Lookup dictionary maps a character to bits (length [1, 256]).
		std::vector<bool> charBits[UCHAR_MAX + 1];

		// Compute frequency array from text.
		std::array<std::size_t, UCHAR_MAX + 1> computeFrequencyFromText(
			std::string const &text) {
			std::array<std::size_t, UCHAR_MAX + 1> frequency{0};
			for (char const &i : text) {
				frequency[reinterpret_cast<unsigned char const &>(i)]++;
			}
			return frequency;
		}

		// Private constructor sets internal pointers to an empty buffer so that
		// underflow/overflow is called every time.
		void initialize() {
			this->setg(&this->getArea, &this->getArea, &this->getArea);
			this->setp(nullptr, nullptr);

			for (short i{0}; i < MAX_NODES; i++) {
				this->children[i] = {-1, -1};
				this->parent[i] = -1;
			}

			this->nextSBumpCs = {
				this->underlying.sbumpc(), this->underlying.sbumpc()};
		}

		// Replenish an empty iBuffer with up to 8 bits, without including any
		// filler bits.
		void replenishIBuffer() {
			if (traits_type::eq_int_type(
						this->nextSBumpCs.first, traits_type::eof())) {
				throw Exception(Error::UNDERLYING_SBUMPC_FAILED);
			}
			int_type bump{this->underlying.sbumpc()};
			if (traits_type::eq_int_type(bump, traits_type::eof())) {
				// If nextSBumpCs are the final two characters, then we must use the
				// second to determine the number of filler bits in the first, and
				// replenish from what is remaining in the first.
				this->iBuffer = {
					static_cast<unsigned char>(this->nextSBumpCs.first) >>
						this->nextSBumpCs.second,
					8 - this->nextSBumpCs.second};
				this->nextSBumpCs = {traits_type::eof(), traits_type::eof()};
			} else {
				// Just replenish from the first nextSBumpC and push the new bump to the
				// back.
				this->iBuffer = {
					static_cast<unsigned char>(this->nextSBumpCs.first), 8};
				this->nextSBumpCs = {this->nextSBumpCs.second, bump};
			}
		}

		// Internal helpers to read/write the next encoded bit/byte to the
		// underlying stream via the buffers. Throws if sbumpc or sputc fail.
		bool readBit() {
			if (this->iBuffer.second == 0) {
				this->replenishIBuffer();
			}
			bool bit{0 != (this->iBuffer.first & (1 << (this->iBuffer.second - 1)))};
			this->iBuffer.second--;
			return bit;
		}
		unsigned char readByte() {
			unsigned char byte{static_cast<unsigned char>(
				this->iBuffer.first << (8 - this->iBuffer.second))};
			unsigned char remainingBits{this->iBuffer.second};
			this->replenishIBuffer();
			this->iBuffer.second = remainingBits;
			return byte | (this->iBuffer.first >> this->iBuffer.second);
		}
		void writeBit(bool bit) {
			this->oBuffer.first = (this->oBuffer.first << 1) + bit;
			this->oBuffer.second++;
			if (this->oBuffer.second == 8) {
				if (traits_type::eq_int_type(
							this->underlying.sputc(this->oBuffer.first),
							traits_type::eof())) {
					throw Exception(Error::UNDERLYING_SPUTC_FAILED);
				}
				this->oBuffer.second = 0;
			}
		}
		void writeByte(unsigned char byte) {
			this->oBuffer.first =
				(this->oBuffer.first << (8 - this->oBuffer.second)) |
				(byte >> this->oBuffer.second);
			if (traits_type::eq_int_type(
						this->underlying.sputc(this->oBuffer.first), traits_type::eof())) {
				throw Exception(Error::UNDERLYING_SPUTC_FAILED);
			}
			this->oBuffer.first = byte;
		}

		// Pre-order traversal to output dictionary to underlying as bit
		// representation.
		void writeDictionaryNode(short curNode) noexcept {
			if (curNode > UCHAR_MAX) {
				// Non-leaf node.
				this->writeBit(false);
				this->writeDictionaryNode(this->children[curNode].first);
				this->writeDictionaryNode(this->children[curNode].second);
			} else {
				// Leaf node.
				this->writeBit(true);
				this->writeByte(static_cast<unsigned char>(curNode));
			}
		}

		// Compute character bits from a tree traversal.
		void computeCharBitsFromNode(
			short curNode,
			std::vector<bool> &bits) noexcept {
			if (curNode > UCHAR_MAX) {
				// Non-leaf node.
				bits.push_back(false);
				this->computeCharBitsFromNode(this->children[curNode].first, bits);
				bits.back() = true;
				this->computeCharBitsFromNode(this->children[curNode].second, bits);
				bits.pop_back();
			} else {
				// Leaf node.
				this->charBits[curNode] = bits;
			}
		}

		void computeCharBits() noexcept {
			std::vector<bool> bits;
			this->computeCharBitsFromNode(this->root, bits);
		}

		public:
		// Build dictionary from frequency array. Optionally (by default) write the
		// dictionary to the underlying stream in bit format as described above.
		HuffmanStreamBuf(
			std::streambuf &underlying,
			std::array<std::size_t, UCHAR_MAX + 1> const &frequency,
			bool writeDictionary = true)
				: underlying(underlying) {
			this->initialize();

			// Push all leaf nodes to priority queue.
			std::priority_queue<
				std::pair<std::size_t, short>,
				std::vector<std::pair<std::size_t, short>>,
				std::greater<std::pair<std::size_t, short>>>
				pq;
			for (std::size_t i{0}; i < frequency.size(); i++) {
				if (frequency[i] > 0) {
					pq.push({frequency[i], static_cast<short>(i)});
				}
			}
			if (pq.size() == 1) {
				throw Exception(Error::MALFORMED_DICTIONARY_ONE_DISTINCT);
			}

			// Begin consolidating nodes, lowest priority first.
			short nextNode{UCHAR_MAX + 1};
			while (pq.size() >= 2) {
				auto left = pq.top();
				pq.pop();
				auto right = pq.top();
				pq.pop();
				this->children[nextNode] = {left.second, right.second};
				this->parent[left.second] = this->parent[right.second] = nextNode;
				pq.push({left.first + right.first, nextNode++});
			}

			// The final node left is the root.
			this->root = pq.top().second;
			this->parent[this->root] = -1;
			this->computeCharBits();

			if (writeDictionary) {
				// Write dictionary to the underlying stream via a pre-order traversal.
				this->writeDictionaryNode(this->root);
			}
		}

		// Build dictionary by scanning a string for frequencies.
		HuffmanStreamBuf(
			std::stringbuf &underlying,
			std::string const &text,
			bool writeDictionary = true)
				: HuffmanStreamBuf(
						underlying,
						this->computeFrequencyFromText(text),
						writeDictionary) {}

		// Read dictionary from first bytes of underlying. Throws on invalid
		// underlying bits.
		HuffmanStreamBuf(std::streambuf &underlying) : underlying(underlying) {
			this->initialize();

			// Traverse the tree while creating nodes. curNode always points to a
			// non-leaf node which still needs at least one child to be traversed.
			short curNode{UCHAR_MAX + 1}, nextNonLeafNode{UCHAR_MAX + 2};
			this->root = curNode;
			this->parent[curNode] = -1;
			this->children[curNode] = {-1, -1};

			// First bit is always 0, since text must have at last 2 distinct
			// characters.
			this->readBit();

			// Each iteration of the while loop either increments nextNonLeafNode or
			// increases number of leaves, which will either eventually succeed or
			// eventually throw.
			while (true) {
				if (!this->readBit()) {
					// If we cannot create the next non-leaf node, the input is malformed.
					if (nextNonLeafNode >= MAX_NODES) {
						throw Exception(Error::MALFORMED_DICTIONARY_TOO_MANY_NODES);
					}

					// Non-leaf node. Assign to either left or right child, based on
					// whether left child is assigned already.
					this->parent[nextNonLeafNode] = curNode;
					if (this->children[curNode].first == -1) {
						this->children[curNode].first = nextNonLeafNode;
					} else {
						this->children[curNode].second = nextNonLeafNode;
					}

					// Descend to fill out that node.
					curNode = nextNonLeafNode++;
					this->children[curNode] = {-1, -1};
				} else {
					// Leaf node. First read its representative character.
					unsigned char idx{this->readByte()};

					// We must encounter each leaf at most once.
					if (this->parent[idx] != -1) {
						throw Exception(Error::MALFORMED_DICTIONARY_LEAF_SEEN_TWICE);
					}

					// Assign to either left or right child, and ascend if both children
					// have now been assigned.
					this->parent[idx] = curNode;
					if (this->children[curNode].first == -1) {
						this->children[curNode].first = idx;
					} else {
						this->children[curNode].second = idx;

						while (curNode != -1 && this->children[curNode].second != -1) {
							curNode = this->parent[curNode];
						}
						if (curNode == -1) {
							// We are done with the tree.
							break;
						}
					}
				}
			}

			this->computeCharBits();
		}

		protected:
		// Read from underlying and decode until we can emit a byte.
		virtual int_type underflow() noexcept override {
			try {
				short curNode{this->root};
				while (curNode > UCHAR_MAX) {
					bool direction{this->readBit()};
					curNode = direction ? this->children[curNode].second
															: this->children[curNode].first;
					// curNode is guaranteed not to be -1 as long as the dictionary was
					// constructed without problem.
				}
				unsigned char byte{static_cast<unsigned char>(curNode)};
				this->getArea = reinterpret_cast<char &>(byte);
				this->setg(&this->getArea, &this->getArea, &this->getArea + 1);
				return traits_type::to_int_type(this->getArea);
			} catch (...) {
				return traits_type::eof();
			}
		}

		// Encodes a byte to oBuffer, writing to underlying as necessary.
		virtual int_type overflow(int_type ch = traits_type::eof()) override {
			if (this->flushed) {
				throw Exception(Error::CANNOT_PUT_AFTER_FLUSH);
			}
			if (!traits_type::eq_int_type(ch, traits_type::eof())) {
				if (this->charBits[ch].empty()) {
					throw Exception(Error::CHARACTER_NOT_IN_DICTIONARY);
				}
				try {
					for (bool const &i : this->charBits[ch]) {
						this->writeBit(i);
					}
				} catch (...) {
					return traits_type::eof();
				}
			}
			return ch;
		}

		// Flushes remaining bits in oBuffer to underlying as a full byte.
		// Writing after sync is called will result in data corruption.
		virtual int sync() override {
			if (this->flushed) {
				throw Exception(Error::CANNOT_PUT_AFTER_FLUSH);
			}
			try {
				unsigned char fillers{
					static_cast<unsigned char>((8 - this->oBuffer.second) % 8)};
				for (unsigned char i{0}; i < fillers; i++) {
					this->writeBit(false);
				}
				this->writeByte(fillers);
				this->underlying.pubsync();
				this->flushed = true;
				return 0;
			} catch (...) {
				return -1;
			}
		}
	};
}

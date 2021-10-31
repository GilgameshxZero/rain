// Segment values with lazy propagation, supporting range queries and range
// updates in O(ln N).
#pragma once

#include <memory>
#include <vector>

#include "algorithm.hpp"

namespace Rain::Algorithm {
	// Segment values with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// RVO helps with heavier Value types, but is not guaranteed. Value must be
	// zero-initialized to be valid. Update must be light-copyable.
	//
	// For parent i, 2i + 1 is the left child, and 2i + 2 is the right child. All
	// left children are odd and all right children are even.
	template <typename Value, typename Update>
	class SegmentTree {
		private:
		// Aggregate values at each node.
		std::vector<Value> values;

		// Size of underlying array.
		std::size_t size,
			// First leaf node.
			firstLeaf;

		// True iff node has a pending lazy update to propagate.
		std::vector<bool> lazy;

		// Lazily-stored updates.
		std::vector<Update> updates;

		public:
		// Segment tree for an underlying array of size size. Must be at least 2.
		SegmentTree(std::size_t const size)
				: values((1_zu << (mostSignificant1BitIdx(size - 1) + 1)) - 1 + size),
					size(size),
					firstLeaf(values.size() - size),
					lazy(firstLeaf, false),
					updates(firstLeaf) {}

		protected:
		// Aggregate values from two children. Aggregating with a
		// default-initialized Value should do nothing. The combined range of the
		// two children is supplied.
		//
		// The internal index of the parent is provided for convenience.
		virtual Value aggregate(
			Value const &left,
			Value const &right,
			std::pair<std::size_t, std::size_t> const &range,
			std::size_t node) = 0;

		// Propagate an update on a parent to its two children.
		//
		// The internal index of the parent is provided for convenience.
		virtual void propagate(
			Update const &update,
			Update &leftChild,
			Update &rightChild,
			std::pair<std::size_t, std::size_t> const &range,
			std::size_t node) = 0;

		// Apply an update to a node, with the range of the node supplied.
		//
		// The internal index of the parent is provided for convenience.
		virtual void apply(
			Value &value,
			Update const &update,
			std::pair<std::size_t, std::size_t> const &range,
			std::size_t node) = 0;

		public:
		// Queries a range, propagating if necessary then aggregating.
		Value query(std::size_t const left, std::size_t const right) {
			return this->query(left, right, 0, {0, this->firstLeaf});
		}

		// Lazy update a range.
		void update(
			std::size_t const left,
			std::size_t const right,
			Update const &update) {
			this->update(left, right, update, 0, {0, this->firstLeaf});
		}

		private:
		// Conditionally propagate a node if it is not a leaf and has an update to
		// propagate.
		void propagate(
			std::size_t node,
			std::pair<std::size_t, std::size_t> const &range) {
			if (node >= this->firstLeaf || !this->lazy[node]) {
				return;
			}

			// Perform the propagation, but take care not to go out of bounds.
			// Propagating to a leaf applies it immediately.
			std::vector<std::unique_ptr<Update>> tmpUpdates;
			Update *chUpdates[2];
			if (node * 2 + 1 < this->firstLeaf) {
				chUpdates[0] = &this->updates[node * 2 + 1];
				this->lazy[node * 2 + 1] = true;
			} else {
				tmpUpdates.emplace_back(new Update{});
				chUpdates[0] = tmpUpdates.back().get();
			}
			if (node * 2 + 2 < this->firstLeaf) {
				chUpdates[1] = &this->updates[node * 2 + 2];
				this->lazy[node * 2 + 2] = true;
			} else {
				tmpUpdates.emplace_back(new Update{});
				chUpdates[1] = tmpUpdates.back().get();
			}
			this->propagate(
				this->updates[node], *chUpdates[0], *chUpdates[1], range, node);
			if (node * 2 + 1 >= this->firstLeaf) {
				this->apply(
					this->values[node * 2 + 1],
					*chUpdates[0],
					{node * 2 + 1 - this->firstLeaf, node * 2 + 1 - this->firstLeaf},
					node * 2 + 1);
			}
			if (node * 2 + 2 >= this->firstLeaf) {
				this->apply(
					this->values[node * 2 + 2],
					*chUpdates[1],
					{node * 2 + 2 - this->firstLeaf, node * 2 + 2 - this->firstLeaf},
					node * 2 + 2);
			}

			// In any case, clear the update at this node so it doesn’t interfere with
			// later propagations.
			this->apply(this->values[node], this->updates[node], range, node);
			this->updates[node] = {};
			this->lazy[node] = false;
		}

		// Internal recursive query. range is the coverage range of the current node
		// and is inclusive.
		Value query(
			std::size_t const left,
			std::size_t const right,
			std::size_t const node,
			std::pair<std::size_t, std::size_t> const &range) {
			if (right < range.first || left > range.second) {
				return {};
			}
			this->propagate(node, range);

			// Base case.
			if (range.first >= left && range.second <= right) {
				return this->values[node];
			}

			std::size_t mid = (range.first + range.second) / 2;
			return this->aggregate(
				this->query(left, right, node * 2 + 1, {range.first, mid}),
				this->query(left, right, node * 2 + 2, {mid + 1, range.second}),
				range,
				node);
		}

		// Internal recursive update.
		void update(
			std::size_t const left,
			std::size_t const right,
			Update const &update,
			std::size_t const node,
			std::pair<std::size_t, std::size_t> const &range) {
			if (right < range.first || left > range.second) {
				return;
			}
			// Propagate even if this node is fully covered, since we don’t have a
			// function to combine two updates.
			this->propagate(node, range);

			// Base case.
			if (range.first >= left && range.second <= right) {
				if (node < this->firstLeaf) {
					this->updates[node] = update;
					this->lazy[node] = true;
				} else {
					this->apply(this->values[node], update, range, node);
				}
			} else {
				std::size_t mid = (range.first + range.second) / 2;
				this->update(left, right, update, node * 2 + 1, {range.first, mid});
				this->update(
					left, right, update, node * 2 + 2, {mid + 1, range.second});

				// Substitute parent value with aggregate after update has been
				// propagated. O(1). Guaranteed at least one child, or else the base
				// case would have triggered.
				this->propagate(node * 2 + 1, {range.first, mid});
				this->propagate(node * 2 + 2, {mid + 1, range.second});
				this->values[node] = this->aggregate(
					this->values[node * 2 + 1],
					node * 2 + 2 < this->values.size() ? this->values[node * 2 + 2]
																						 : Value{},
					range,
					node);
			}
		}
	};
}

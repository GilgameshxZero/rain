// Segment tree with lazy propagation, supporting range queries and range
// updates in O(ln N).
#pragma once

#include <memory>
#include <vector>

#include "algorithm.hpp"

namespace Rain::Algorithm {
	// Segment tree with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// RVO helps with heavier Result types, but is not guaranteed. Value must be
	// zero-initialized to be valid. Update must be light-copyable.
	//
	// Index 0 is unused. For parent i, 2i is the left child and 2i + 1 is the
	// right child.
	template <typename _Value, typename _Update, typename _Result = _Value>
	class SegmentTree {
		protected:
		using Value = _Value;
		using Update = _Update;
		using Result = _Result;

		// Aggregate values at each node.
		std::vector<Value> values;

		// True iff node has a pending lazy update to propagate.
		std::vector<bool> lazy;

		// Lazily-stored updates.
		std::vector<Update> updates;

		public:
		// Segment tree for a segment array of size size.
		SegmentTree(std::size_t const size)
				: values(1_zu << (mostSignificant1BitIdx(size - 1) + 2)),
					lazy(values.size(), false),
					updates(values.size()) {}

		protected:
		// Aggregate values from two children while retracing an update. Aggregating
		// with a default-initialized Value should do nothing.
		virtual void aggregate(
			std::size_t const node,
			typename std::vector<Value>::reference value,
			Value const &left,
			Value const &right,
			std::pair<std::size_t, std::size_t> const &range) = 0;

		// Aggregate two results from queries on children. Aggregating with a
		// default-initialized Result should do nothing.
		virtual Result aggregate(
			std::size_t const node,
			Result const &left,
			Result const &right,
			std::pair<std::size_t, std::size_t> const &range) = 0;

		// Propagate an update on a parent to its two children. Lazy bits for the
		// children are set beforehand, but can be unset in the function.
		virtual void split(
			std::size_t const node,
			Update const &update,
			typename std::vector<Update>::reference left,
			typename std::vector<Update>::reference right,
			std::pair<std::size_t, std::size_t> const &range) = 0;

		// Convert a Value at a leaf node to a Result for base case queries.
		virtual Result convert(std::size_t const node, Value const &value) = 0;

		// Apply an update fully to a lazy node.
		virtual void apply(
			std::size_t const node,
			typename std::vector<Value>::reference value,
			Update const &update,
			std::pair<std::size_t, std::size_t> const &range) = 0;

		public:
		// Queries a range, propagating if necessary then aggregating.
		Result query(std::size_t const left, std::size_t const right) {
			return this->query(1, left, right, {0, this->values.size() / 2 - 1});
		}

		// Lazy update a range.
		void update(
			std::size_t const left,
			std::size_t const right,
			Update const &update) {
			this->update(1, left, right, update, {0, this->values.size() / 2 - 1});
		}

		private:
		// Conditionally propagate a node if it is not a leaf and has an update to
		// propagate.
		void propagate(
			std::size_t const node,
			std::pair<std::size_t, std::size_t> const &range) {
			if (!this->lazy[node]) {
				return;
			}

			// Propagating on a leaf applies it immediately.
			if (node < this->values.size() / 2) {
				this->lazy[node * 2] = this->lazy[node * 2 + 1] = true;
				this->split(
					node,
					this->updates[node],
					this->updates[node * 2],
					this->updates[node * 2 + 1],
					range);
			}

			// Clear the update at this node so it doesn’t interfere with later
			// propagations.
			this->apply(node, this->values[node], this->updates[node], range);
			this->updates[node] = {};
			this->lazy[node] = false;
		}

		// Internal recursive query. range is the coverage range of the current node
		// and is inclusive.
		Result query(
			std::size_t const node,
			std::size_t const left,
			std::size_t const right,
			std::pair<std::size_t, std::size_t> const &range) {
			if (right < range.first || left > range.second) {
				return {};
			}
			this->propagate(node, range);

			// Base case.
			if (range.first >= left && range.second <= right) {
				return this->convert(node, this->values[node]);
			}

			std::size_t mid = (range.first + range.second) / 2;
			return this->aggregate(
				node,
				this->query(node * 2, left, right, {range.first, mid}),
				this->query(node * 2 + 1, left, right, {mid + 1, range.second}),
				range);
		}

		// Internal recursive update.
		void update(
			std::size_t const node,
			std::size_t const left,
			std::size_t const right,
			Update const &update,
			std::pair<std::size_t, std::size_t> const &range) {
			if (right < range.first || left > range.second) {
				return;
			}
			// Propagate even if this node is fully covered, since we don’t have a
			// function to combine two updates.
			this->propagate(node, range);

			// Base case.
			if (range.first >= left && range.second <= right) {
				// This node is already non-lazy since it was just propagated.
				this->updates[node] = update;
				this->lazy[node] = true;
			} else {
				std::size_t mid = (range.first + range.second) / 2;
				this->update(node * 2, left, right, update, {range.first, mid});
				this->update(
					node * 2 + 1, left, right, update, {mid + 1, range.second});

				// Substitute parent value with aggregate after update has been
				// propagated. O(1). Guaranteed at least one child, or else the base
				// case would have triggered.
				this->propagate(node * 2, {range.first, mid});
				this->propagate(node * 2 + 1, {mid + 1, range.second});
				this->aggregate(
					node,
					this->values[node],
					this->values[node * 2],
					this->values[node * 2 + 1],
					range);
			}
		}
	};
}

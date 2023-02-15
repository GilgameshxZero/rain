// Segment tree with lazy propagation, supporting range queries and range
// updates in O(ln N).
#pragma once

#include "../literal.hpp"
#include "bit-manipulators.hpp"

#include <vector>

namespace Rain::Algorithm {
	// Segment tree with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// RVO helps with heavier Result types, but is not guaranteed. Value must be
	// zero-initialized to be valid. Update must be light-copyable.
	//
	// Index 0 is unused. For parent i, 2i is the left child and 2i + 1 is the
	// right child. The queries and updates themselves are inclusive and 0-indexed
	// on the represented range.
	//
	// Further performance can be had with manual inlining of the five functions,
	// which are typically not inlined by the compiler due to their virtual-ness.
	// Forced initialization of underlying vectors can be replaced with
	// std::array, and on GCC, loop unrolling may be turned on for further
	// optimization. Removing functions which support lazy propagation will
	// further speed up this implementation.
	//
	// Additionally, consider using the non-lazy version of this tree if range
	// updates are not required.
	//
	// The default Value should be one which serves as an identity when acted upon
	// by aggregate, apply, and propagate, and its converted Result should also be
	// an identity w.r.t aggregate.
	template <
		typename Value,
		typename Update,
		typename Result,
		Value DEFAULT_VALUE,
		// Aggregate values from two children while retracing an update. Aggregating
		// with a default Value should do nothing.
		void (*aggregateValues)(
			std::size_t const,
			typename std::vector<Value>::reference,
			Value const &,
			Value const &,
			std::pair<std::size_t, std::size_t> const &),
		// Aggregate two results from queries on children. Aggregating with a
		// Result converted from a default Value should do nothing.
		Result (*aggregateResults)(
			std::size_t const,
			Result const &,
			Result const &,
			std::pair<std::size_t, std::size_t> const &),
		// Propagate an update on a parent to its two children. Lazy bits for the
		// children are set beforehand, but can be unset in the function.
		void (*split)(
			std::size_t const,
			Update const &,
			typename std::vector<Update>::reference,
			typename std::vector<Update>::reference,
			std::pair<std::size_t, std::size_t> const &),
		// Apply an update fully to a lazy node.
		void (*apply)(
			std::size_t const,
			typename std::vector<Value>::reference,
			Update const &,
			std::pair<std::size_t, std::size_t> const &),
		// Convert a Value at a leaf node to a Result for base case queries.
		Result (*convert)(std::size_t const, Value const &)>
	class SegmentTreeLazy {
		protected:
		// Aggregate values at each node.
		std::vector<Value> values;

		// True iff node has a pending lazy update to propagate.
		std::vector<bool> lazy;

		// Lazily-stored updates.
		std::vector<Update> updates;

		public:
		// Segment tree for a segment array of size size.
		SegmentTreeLazy(std::size_t const size)
				: values(1_zu << (mostSignificant1BitIdx(size - 1) + 2), DEFAULT_VALUE),
					lazy(values.size(), false),
					updates(values.size()) {}

		// Queries an inclusive range, propagating if necessary then aggregating.
		inline Result query(std::size_t const left, std::size_t const right) {
			return this->query(1, left, right, {0, this->values.size() / 2 - 1});
		}

		// Lazy update an inclusive range.
		inline void update(
			std::size_t const left,
			std::size_t const right,
			Update const &update) {
			this->update(1, left, right, update, {0, this->values.size() / 2 - 1});
		}

		private:
		// Conditionally propagate a node if it is not a leaf and has an update to
		// propagate.
		inline void propagate(
			std::size_t const node,
			std::pair<std::size_t, std::size_t> const &range) {
			if (!this->lazy[node]) {
				return;
			}

			// Propagating on a leaf applies it immediately. Otherwise, split the
			// update to children.
			if (node < this->values.size() / 2) {
				this->lazy[node * 2] = this->lazy[node * 2 + 1] = true;
				split(
					node,
					this->updates[node],
					this->updates[node * 2],
					this->updates[node * 2 + 1],
					range);
			}

			// Clear the update at this node so it doesn’t interfere with later
			// propagations.
			apply(node, this->values[node], this->updates[node], range);
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
				return DEFAULT_VALUE;
			}
			this->propagate(node, range);

			// Base case.
			if (range.first >= left && range.second <= right) {
				return convert(node, this->values[node]);
			}

			std::size_t mid{(range.first + range.second) / 2};
			return aggregateResults(
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
				std::size_t mid{(range.first + range.second) / 2};
				this->update(node * 2, left, right, update, {range.first, mid});
				this->update(
					node * 2 + 1, left, right, update, {mid + 1, range.second});

				// Substitute parent value with aggregate after update has been
				// propagated. O(1). Guaranteed at least one child, or else the base
				// case would have triggered.
				this->propagate(node * 2, {range.first, mid});
				this->propagate(node * 2 + 1, {mid + 1, range.second});
				aggregateValues(
					node,
					this->values[node],
					this->values[node * 2],
					this->values[node * 2 + 1],
					range);
			}
		}
	};
}

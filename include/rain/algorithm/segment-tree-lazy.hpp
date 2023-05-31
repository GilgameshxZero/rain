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
	// Based on <https://codeforces.com/blog/entry/18051>. Earlier iterations of
	// this data structure have higher constant factor but enable more intuitive
	// modifications. This policy-based structure requires a policy of the
	// following interface:
	//
	// static constexpr Value DEFAULT_VALUE: Identity values at creation and
	// aggregation.
	//
	// static constexpr Update DEFAULT_UPDATE: Identify value of an update.
	//
	// static void apply(Value &value, Update const &update, std::size_t range):
	// Fully apply an update to a node.
	//
	// static Result aggregate(Result const &left, Result const &right):
	// Aggregate two results from queries on children. Aggregating with a Result
	// converted from a default Value should do nothing.
	//
	// static void retrace(Value &value, Value const &left, Value const &right,
	// std::size_t range): Aggregate values from two children while retracing an
	// update. Aggregating with a default Value should do nothing.
	//
	// static void split(Update const &update, Update &left, Update &right,
	// std::size_t range): Split a lazy update into its children updates.
	template <typename Policy>
	class SegmentTreeLazy {
		public:
		using Value = typename Policy::Value;
		using Update = typename Policy::Update;
		using Result = typename Policy::Result;

		protected:
		// Aggregate values at each node.
		std::vector<Value> values;

		// True iff node has a pending lazy update to propagate.
		std::vector<bool> lazy;

		// Lazily-stored updates.
		std::vector<Update> updates;

		// Height of the highest node in the tree.
		std::size_t const HEIGHT;

		public:
		// Segment tree for a segment array of size size.
		SegmentTreeLazy(std::size_t const size)
				: values(2 * size, Policy::DEFAULT_VALUE),
					lazy(values.size(), false),
					updates(values.size(), Policy::DEFAULT_UPDATE),
					HEIGHT{mostSignificant1BitIdx(values.size())} {}

		protected:
		// Propagate all ancestors of nodes in a given inclusive underlying range.
		void propagate(std::size_t left, std::size_t right) {
			std::size_t level{this->HEIGHT}, range{1_zu << (this->HEIGHT - 1)};
			for (left += this->values.size() / 2, right += this->values.size() / 2;
					 level > 0;
					 --level, range >>= 1) {
				for (std::size_t i{left >> level}; i <= (right >> level); ++i) {
					if (this->lazy[i]) {
						Policy::apply(this->values[i * 2], this->updates[i], range);
						Policy::apply(this->values[i * 2 + 1], this->updates[i], range);
						Policy::split(
							this->updates[i],
							this->updates[i * 2],
							this->updates[i * 2 + 1],
							range);
						this->lazy[i * 2] = this->lazy[i * 2 + 1] = true;

						this->updates[i] = Policy::DEFAULT_UPDATE;
						this->lazy[i] = false;
					}
				}
			}
		}

		public:
		// Queries an inclusive range, propagating if necessary then aggregating.
		Result query(std::size_t left, std::size_t right) {
			this->propagate(left, left);
			this->propagate(right, right);
			Value resLeft{Policy::DEFAULT_VALUE}, resRight{Policy::DEFAULT_VALUE};
			for (left += this->values.size() / 2,
					 right += this->values.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2) {
				if (left % 2 == 1) {
					resLeft = Policy::aggregate(resLeft, this->values[left++]);
				}
				if (right % 2 == 1) {
					resRight = Policy::aggregate(this->values[--right], resRight);
				}
			}
			return Policy::aggregate(resLeft, resRight);
		}

		// Lazy update an inclusive range.
		void update(std::size_t left, std::size_t right, Update const &update) {
			this->propagate(left, left);
			this->propagate(right, right);
			// Only retrace updates once left or right node has been changed.
			bool changedLeft{false}, changedRight{false};
			std::size_t range{1};
			for (left += this->values.size() / 2,
					 right += this->values.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2, range *= 2) {
				if (changedLeft) {
					Policy::retrace(
						this->values[left - 1],
						this->values[left * 2 - 2],
						this->values[left * 2 - 1],
						range);
				}
				if (changedRight) {
					Policy::retrace(
						this->values[right],
						this->values[right * 2],
						this->values[right * 2 + 1],
						range);
				}
				if (left % 2 == 1) {
					Policy::apply(this->values[left++], update, range);
					this->lazy[left - 1] = true;
					this->updates[left - 1] = update;
					changedLeft = true;
				}
				if (right % 2 == 1) {
					Policy::apply(this->values[--right], update, range);
					this->lazy[right] = true;
					this->updates[right] = update;
					changedRight = true;
				}
			}
			for (left--; right > 0; left /= 2, right /= 2, range *= 2) {
				if (changedLeft) {
					Policy::retrace(
						this->values[left],
						this->values[left * 2],
						this->values[left * 2 + 1],
						range);
				}
				if (changedRight && (!changedLeft || left != right)) {
					Policy::retrace(
						this->values[right],
						this->values[right * 2],
						this->values[right * 2 + 1],
						range);
				}
			}
		}
	};
}

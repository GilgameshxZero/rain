// A special optimization of the segment tree without support for lazy
// propagation. Only support point updates instead of range updates, but has a
// much faster constant factor.
#pragma once

#include "../literal.hpp"
#include "bit-manipulators.hpp"

#include <vector>

namespace Rain::Algorithm {
	// Segment tree without lazy propagation nor range updates.
	template <
		typename Value,
		typename Update,
		typename Result,
		Value DEFAULT_VALUE,
		// Aggregate values from two children while retracing an update. Aggregating
		// with a default Value should do nothing.
		void (*retrace)(
			typename std::vector<Value>::reference,
			Value const &,
			Value const &),
		// Aggregate two results from queries on children. Aggregating with a
		// Result converted from a default Value should do nothing.
		Result (*aggregate)(Result const &, Result const &),
		// Apply an update fully to a node.
		void (*apply)(typename std::vector<Value>::reference, Update const &)>
	class SegmentTree {
		protected:
		// Aggregate values at each node.
		std::vector<Value> values;

		public:
		// Segment tree for a segment array of size size.
		SegmentTree(std::size_t const size)
				: values(
						1_zu << (mostSignificant1BitIdx(size - 1) + 2),
						DEFAULT_VALUE) {}

		// Queries an inclusive range.
		Result query(std::size_t left, std::size_t right) {
			Value resLeft{DEFAULT_VALUE}, resRight{DEFAULT_VALUE};
			for (left += this->values.size() / 2,
					 right += this->values.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2) {
				if (left % 2 == 1) {
					resLeft = aggregate(resLeft, this->values[left++]);
				}
				if (right % 2 == 1) {
					resRight = aggregate(resRight, this->values[--right]);
				}
			}
			return aggregate(resLeft, resRight);
		}

		// Point update an index.
		void update(std::size_t idx, Update const &update) {
			idx += this->values.size() / 2;
			apply(this->values[idx], update);
			for (idx /= 2; idx >= 1; idx /= 2) {
				retrace(
					this->values[idx], this->values[idx * 2], this->values[idx * 2 + 1]);
			}
		}
	};
}

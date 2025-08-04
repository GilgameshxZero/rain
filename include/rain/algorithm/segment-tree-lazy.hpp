// Segment tree with lazy propagation, supporting range queries and range
// updates in O(ln N).
#pragma once

#include "../literal.hpp"
#include "bit-manipulators.hpp"

#include <type_traits>
#include <vector>

namespace Rain::Algorithm {
	template <typename = void>
	class SegmentTreeLazy;

	template <>
	class SegmentTreeLazy<void> {
		private:
		// SFINAE base class which conditionally defines DEFAULT_VALUE.
		template <typename Value, typename = void>
		class PolicyBaseDefaultValue {};

		template <typename Value>
		class PolicyBaseDefaultValue<
			Value,
			typename std::enable_if<
				std::is_default_constructible<Value>::value>::type> {
			protected:
			Value const DEFAULT_VALUE{};
		};

		// SFINAE base class which conditionally defines DEFAULT_UPDATE.
		template <typename Update, typename = void>
		class PolicyBaseDefaultUpdate {};

		template <typename Update>
		class PolicyBaseDefaultUpdate<
			Update,
			typename std::enable_if<
				std::is_default_constructible<Update>::value>::type> {
			protected:
			Update const DEFAULT_UPDATE{};
		};

		// SFINAE base class which conditionally defines combine. While MSVC allows
		// placing this function with SFINAE directly in the policy class, other
		// compilers do not, and so we compromise and place it outside, and inherit
		// it.
		template <typename Update, typename = void>
		class PolicyBaseCombine {};

		template <typename Update>
		class PolicyBaseCombine<
			Update,
			typename std::void_t<
				decltype(std::declval<Update &>() += std::declval<Update const &>())>> {
			protected:
			// Stack an update atop an existing, potentially non-empty update, which
			// will be used lazily later. Must be associative but may not be
			// commutative.
			inline void combine(Update &current, Update const &update, std::size_t) {
				current += update;
			}
		};

		// SFINAE base class which conditionally defines retrace.
		template <typename Value, typename = void>
		class PolicyBaseRetrace {};

		template <typename Value>
		class PolicyBaseRetrace<
			Value,
			typename std::void_t<
				decltype(std::declval<Value &>() = std::declval<Value const &>() + std::declval<Value const &>())>> {
			protected:
			// Aggregate values from two children while retracing an update.
			// Aggregating with a default Value should do nothing. Must be associative
			// but may not be commutative.
			inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				std::size_t) {
				value = left + right;
			}
		};

		// SFINAE base class which conditionally defines apply.
		template <typename Value, typename Update, typename = void>
		class PolicyBaseApply {};

		template <typename Value, typename Update>
		class PolicyBaseApply<
			Value,
			Update,
			typename std::void_t<
				decltype(std::declval<Value &>() += std::declval<Update const &>() * std::declval<std::size_t>())>> {
			protected:
			// Fully apply an update to a node. The node may not be a leaf.
			inline void apply(Value &value, Update const &update, std::size_t range) {
				value += update * range;
			}
		};

		// SFINAE base class which conditionally defines aggregate.
		template <typename Result, typename = void>
		class PolicyBaseAggregate {};

		template <typename Result>
		class PolicyBaseAggregate<
			Result,
			typename std::enable_if<std::is_constructible<
				Result,
				decltype(std::declval<Result const &>() + std::declval<Result const &>())>::
																value>::type> {
			protected:
			// Aggregate two results from queries on children. Aggregating with a
			// Result converted from a default Value should do nothing. Must be
			// associative but may not be commutative.
			inline Result aggregate(Result const &left, Result const &right) {
				return {left + right};
			}
		};

		public:
		// Default policy for SegmentTreeLazy, which has functions disabled by
		// SFINAE if they do not support expected operations. In that case, the
		// client should inherit the enabled parts of this disabled policy and
		// re-implement the disabled functions.
		//
		// This default policy represents a sum tree.
		template <
			typename ValueType,
			typename UpdateType = ValueType,
			typename ResultType = ValueType>
		class Policy : protected PolicyBaseDefaultValue<ValueType>,
									 protected PolicyBaseDefaultUpdate<UpdateType>,
									 protected PolicyBaseCombine<UpdateType>,
									 protected PolicyBaseRetrace<ValueType>,
									 protected PolicyBaseApply<ValueType, UpdateType>,
									 protected PolicyBaseAggregate<ResultType> {
			protected:
			// Expose typenames to subclasses (SegmentTreeLazy).
			using Value = ValueType;
			using Update = UpdateType;
			using Result = ResultType;

			// Convert the value at a node to a result. The node may not be a leaf.
			template <
				bool isConstructible = std::is_constructible<Result, Value>::value,
				typename std::enable_if<isConstructible>::type * = nullptr>
			inline Result convert(Value const &value, std::size_t) {
				return {value};
			}
		};
	};

	// Segment tree with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// Loosely based on <https://codeforces.com/blog/entry/18051>. Earlier
	// iterations of this data structure have higher constant factor but enable
	// more intuitive modifications. Inherit and modify the provided default
	// policy to implement custom behavior.
	template <typename Policy>
	class SegmentTreeLazy : protected Policy {
		public:
		using Value = typename Policy::Value;
		using Update = typename Policy::Update;
		using Result = typename Policy::Result;

		protected:
		// Aggregate values at each node. Index 0 is unused.
		std::vector<Value> values;

		// True iff node has a pending lazy update to propagate to its children. The
		// update has already been applied to the node itself.
		std::vector<bool> lazy;

		// Lazily-stored updates.
		std::vector<Update> updates;

		// Height of the highest node in the tree.
		std::size_t const HEIGHT;

		public:
		// Segment tree for a segment array of size size.
		SegmentTreeLazy(std::size_t const size)
				: values(2 * size, this->DEFAULT_VALUE),
					lazy(values.size(), false),
					updates(values.size(), this->DEFAULT_UPDATE),
					HEIGHT{mostSignificant1BitIdx(values.size())} {}

		protected:
		// Propagate all ancestors of nodes in a given inclusive underlying range.
		// After this, no ancestor of any node in this range should have a queued
		// update.
		void propagate(std::size_t left, std::size_t right) {
			std::size_t level{this->HEIGHT}, range{1_zu << (this->HEIGHT - 1)};
			for (left += this->values.size() / 2, right += this->values.size() / 2;
					 level > 0;
					 --level, range /= 2) {
				for (std::size_t i{left >> level}; i <= (right >> level); ++i) {
					if (this->lazy[i]) {
						this->apply(this->values[i * 2], this->updates[i], range);
						this->apply(this->values[i * 2 + 1], this->updates[i], range);
						this->combine(this->updates[i * 2], this->updates[i], range);
						this->combine(this->updates[i * 2 + 1], this->updates[i], range);
						this->lazy[i * 2] = this->lazy[i * 2 + 1] = true;

						this->updates[i] = this->DEFAULT_UPDATE;
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
			Result resLeft{this->convert(this->DEFAULT_VALUE, 1)},
				resRight{this->convert(this->DEFAULT_VALUE, 1)};
			std::size_t range{1};
			for (left += this->values.size() / 2,
					 right += this->values.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2, range *= 2) {
				if (left % 2 == 1) {
					resLeft = this->aggregate(
						resLeft, this->convert(this->values[left++], range));
				}
				if (right % 2 == 1) {
					resRight = this->aggregate(
						this->convert(this->values[--right], range), resRight);
				}
			}
			return this->aggregate(resLeft, resRight);
		}

		// Lazy update an inclusive range. The updated will be applied identically
		// to all nodes in the range, save for differences based on the depth of the
		// node (which will be expressed via the std::size_t range parameter).
		void update(std::size_t left, std::size_t right, Update const &update) {
			// We must propagate here because retrace expects non-lazy nodes to store
			// the value.
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
					this->retrace(
						this->values[left - 1],
						this->values[left * 2 - 2],
						this->values[left * 2 - 1],
						range);
				}
				if (changedRight) {
					this->retrace(
						this->values[right],
						this->values[right * 2],
						this->values[right * 2 + 1],
						range);
				}
				if (left % 2 == 1) {
					this->apply(this->values[left++], update, range);
					this->combine(this->updates[left - 1], update, range);
					this->lazy[left - 1] = true;
					changedLeft = true;
				}
				if (right % 2 == 1) {
					this->apply(this->values[--right], update, range);
					this->combine(this->updates[right], update, range);
					this->lazy[right] = true;
					changedRight = true;
				}
			}
			for (left--; right > 0; left /= 2, right /= 2, range *= 2) {
				if (changedLeft) {
					this->retrace(
						this->values[left],
						this->values[left * 2],
						this->values[left * 2 + 1],
						range);
				}
				if (changedRight && (!changedLeft || left != right)) {
					this->retrace(
						this->values[right],
						this->values[right * 2],
						this->values[right * 2 + 1],
						range);
				}
			}
		}
	};
}

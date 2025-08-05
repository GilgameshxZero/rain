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
		// SFINAE base class which conditionally defines defaultValue() and
		// defaultResult().
		template <typename Value, typename Result, typename = void>
		class PolicyBaseDefaultValueResult {};

		template <typename Value, typename Result>
		class PolicyBaseDefaultValueResult<
			Value,
			Result,
			typename std::enable_if<
				std::is_default_constructible<Value>::value>::type> {
			protected:
			inline Value defaultValue() { return {}; }

			template <
				bool isConstructible = std::is_constructible<Result, Value>::value,
				typename std::enable_if<isConstructible>::type * = nullptr>
			inline Result defaultResult() {
				return {this->defaultValue()};
			}
		};

		// SFINAE base class which conditionally defines defaultUpdate().
		template <typename Update, typename = void>
		class PolicyBaseDefaultUpdate {};

		template <typename Update>
		class PolicyBaseDefaultUpdate<
			Update,
			typename std::enable_if<
				std::is_default_constructible<Update>::value>::type> {
			protected:
			inline Update defaultUpdate() { return {}; }
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

		// SFINAE base class which conditionally defines retrace. retrace is
		// slightly problematic for segment trees which store non-trivial values at
		// each node. In such a case, a variant of retrace should be used which
		// applies the update directly.
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
		template <typename Result, typename Query, typename = void>
		class PolicyBaseAggregate {};

		template <typename Result, typename Query>
		class PolicyBaseAggregate<
			Result,
			Query,
			typename std::enable_if<std::is_constructible<
				Result,
				decltype(std::declval<Result const &>() + std::declval<Result const &>())>::
																value>::type> {
			protected:
			// Aggregate two results from queries on children. Aggregating with a
			// Result converted from a default Value should do nothing. Must be
			// associative but may not be commutative.
			inline Result
			aggregate(Result const &left, Result const &right, Query const &) {
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
			typename ResultType = ValueType,
			typename QueryType = std::nullptr_t>
		class Policy
				: protected PolicyBaseDefaultValueResult<ValueType, ResultType>,
					protected PolicyBaseDefaultUpdate<UpdateType>,
					protected PolicyBaseCombine<UpdateType>,
					protected PolicyBaseRetrace<ValueType>,
					protected PolicyBaseApply<ValueType, UpdateType>,
					protected PolicyBaseAggregate<ResultType, QueryType> {
			protected:
			// Expose typenames to subclasses (SegmentTreeLazy).
			using Value = ValueType;
			using Update = UpdateType;
			using Result = ResultType;
			using Query = QueryType;

			// Convert the value at a node to a result. The node may not be a leaf.
			template <
				bool isConstructible = std::is_constructible<Result, Value>::value,
				typename std::enable_if<isConstructible>::type * = nullptr>
			inline Result convert(Value const &value, Query const &, std::size_t) {
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
		using Query = typename Policy::Query;

		private:
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
				: values(2 * size, this->defaultValue()),
					lazy(this->values.size(), false),
					updates(this->values.size(), this->defaultUpdate()),
					HEIGHT{mostSignificant1BitIdx(this->values.size())} {}

		// Segment tree with all leaf nodes moved in, and the others constructed in
		// order. This minimizes build time by a constant factor.
		SegmentTreeLazy(std::vector<Value> &&values)
				: values(values.size(), this->defaultValue()),
					lazy(2 * this->values.size(), false),
					updates(2 * this->values.size(), this->defaultUpdate()),
					HEIGHT{mostSignificant1BitIdx(2 * this->values.size())} {
			this->values.insert(
				this->values.end(),
				std::make_move_iterator(values.begin()),
				std::make_move_iterator(values.end()));
			for (std::size_t level{1}, range{2_zu}; level < this->HEIGHT;
					 level++, range *= 2) {
				for (std::size_t i{values.size() >> level};
						 i < (values.size() >> (level - 1));
						 i++) {
					this->retrace(
						this->values[i],
						this->values[i * 2],
						this->values[i * 2 + 1],
						range);
				}
			}
		}

		private:
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

						this->updates[i] = this->defaultUpdate();
						this->lazy[i] = false;
					}
				}
			}
		}

		public:
		// Queries an inclusive range, propagating if necessary then aggregating.
		// Take an optional query parameter `query` which will be used in convert
		// and aggregate of the results and values.
		Result query(std::size_t left, std::size_t right, Query const &query = {}) {
			this->propagate(left, left);
			this->propagate(right, right);
			Result resLeft{this->defaultResult()}, resRight{this->defaultResult()};
			std::size_t range{1};
			for (left += this->values.size() / 2,
					 right += this->values.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2, range *= 2) {
				if (left % 2 == 1) {
					resLeft = this->aggregate(
						resLeft, this->convert(this->values[left++], query, range), query);
				}
				if (right % 2 == 1) {
					resRight = this->aggregate(
						this->convert(this->values[--right], query, range),
						resRight,
						query);
				}
			}
			return this->aggregate(resLeft, resRight, query);
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

	template <typename ValueType>
	class SegmentTreeLazySumPolicy {
		protected:
		using Value = ValueType;
		using Update = ValueType;
		using Result = ValueType;
		using Query = std::nullptr_t;

		inline Value defaultValue() { return {}; }
		inline Update defaultUpdate() { return {}; }
		inline Result defaultResult() { return {}; }
		inline void combine(Update &current, Update const &update, std::size_t) {
			current += update;
		}
		inline void
		retrace(Value &value, Value const &left, Value const &right, std::size_t) {
			value = left + right;
		}
		inline void apply(Value &value, Update const &update, std::size_t range) {
			value += update * static_cast<Update>(range);
		}
		inline Result
		aggregate(Result const &left, Result const &right, Query const &) {
			return left + right;
		}
		inline Result convert(Value const &value, Query const &, std::size_t) {
			return {value};
		}
	};

	template <typename ValueType>
	using SegmentTreeLazySum =
		SegmentTreeLazy<SegmentTreeLazySumPolicy<ValueType>>;

	template <typename ValueType>
	class SegmentTreeLazyMinPolicy {
		protected:
		using Value = ValueType;
		using Update = ValueType;
		using Result = ValueType;
		using Query = std::nullptr_t;

		inline Value defaultValue() { return {}; }
		inline Update defaultUpdate() { return {}; }
		inline Result defaultResult() { return std::numeric_limits<Result>::max(); }
		inline void combine(Update &current, Update const &update, std::size_t) {
			current += update;
		}
		inline void
		retrace(Value &value, Value const &left, Value const &right, std::size_t) {
			value = std::min(left, right);
		}
		inline void apply(Value &value, Update const &update, std::size_t) {
			value += update;
		}
		inline Result
		aggregate(Result const &left, Result const &right, Query const &) {
			return std::min(left, right);
		}
		inline Result convert(Value const &value, Query const &, std::size_t) {
			return {value};
		}
	};

	template <typename ValueType>
	using SegmentTreeLazyMin =
		SegmentTreeLazy<SegmentTreeLazyMinPolicy<ValueType>>;

	template <typename ValueType>
	class SegmentTreeLazyMaxPolicy {
		protected:
		using Value = ValueType;
		using Update = ValueType;
		using Result = ValueType;
		using Query = std::nullptr_t;

		inline Value defaultValue() { return {}; }
		inline Update defaultUpdate() { return {}; }
		inline Result defaultResult() { return std::numeric_limits<Result>::min(); }
		inline void combine(Update &current, Update const &update, std::size_t) {
			current += update;
		}
		inline void
		retrace(Value &value, Value const &left, Value const &right, std::size_t) {
			value = std::max(left, right);
		}
		inline void apply(Value &value, Update const &update, std::size_t) {
			value += update;
		}
		inline Result
		aggregate(Result const &left, Result const &right, Query const &) {
			return std::max(left, right);
		}
		inline Result convert(Value const &value, Query const &, std::size_t) {
			return {value};
		}
	};

	template <typename ValueType>
	using SegmentTreeLazyMax =
		SegmentTreeLazy<SegmentTreeLazyMaxPolicy<ValueType>>;
}

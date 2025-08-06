// Segment tree with lazy propagation, supporting range queries and range
// updates in O(ln N).
#pragma once

#include "../error/exception.hpp"
#include "../literal.hpp"
#include "bit-manipulators.hpp"
#include "fenwick.hpp"

#include <cassert>
#include <type_traits>
#include <vector>
#include <map>

namespace Rain::Algorithm {
	template <typename = std::nullptr_t>
	class SegmentTreeLazy;

	template <>
	class SegmentTreeLazy<std::nullptr_t> {
		public:
		// Exceptions are defined on the default template.
		enum Error { NONE, NOT_IMPLEMENTED_POLICY, ITERATOR_INVALID };
		class ErrorCategory : public std::error_category {
			public:
			char const *name() const noexcept {
				return "Rain::Algorithm::SegmentTreeLazy";
			}
			std::string message(int error) const noexcept {
				switch (error) {
					case Error::NONE:
						return "None.";
					case Error::NOT_IMPLEMENTED_POLICY:
						return "An unimplemented Policy function was called.";
					case Error::ITERATOR_INVALID:
						return "The requested iterator is invalid.";
					default:
						return "Generic.";
				}
			}
		};
		typedef Rain::Error::Exception<Error, ErrorCategory> Exception;

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
			public:
			static inline Value defaultValue() { return {}; }

			template <
				bool isConstructible = std::is_constructible<Result, Value>::value,
				typename std::enable_if<isConstructible>::type * = nullptr>
			static inline Result defaultResult() {
				return {defaultValue()};
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
			public:
			static inline Update defaultUpdate() { return {}; }
		};

		public:
		// Default policy for SegmentTreeLazy, which has functions disabled by
		// SFINAE if they do not support expected operations. In that case, the
		// client should inherit the enabled parts of this disabled policy and
		// re-implement the disabled functions.
		template <
			typename ValueType,
			typename UpdateType = ValueType,
			typename ResultType = ValueType,
			typename QueryType = std::nullptr_t>
		class PolicyBase
				: public PolicyBaseDefaultValueResult<ValueType, ResultType>,
					public PolicyBaseDefaultUpdate<UpdateType> {
			public:
			// Expose typenames to subclasses (SegmentTreeLazy).
			using Value = ValueType;
			using Update = UpdateType;
			using Result = ResultType;
			using Query = QueryType;

			// Convert the value at a node to a result. The node may not be a leaf.
			template <
				bool isConstructible = std::is_constructible<Result, Value>::value,
				typename std::enable_if<isConstructible>::type * = nullptr>
			static inline Result
			convert(Value const &value, Query const &, std::size_t) {
				return {value};
			}

			// Combine and build are not used in every segtree, and the default is to
			// throw if used.
			static inline void combine(Update &current, Update const &update) {
				throw Exception(Error::NOT_IMPLEMENTED_POLICY);
			}
			static inline void
			build(Value &value, Value const &left, Value const &right) {
				throw Exception(Error::NOT_IMPLEMENTED_POLICY);
			}
		};

		// Wraps a policy to implement a persistent segment tree via the fat-node
		// technique. Range updates are somewhat dangerous because lazy propagation
		// may cause some updates to not be stored in the history. I do not believe
		// lazy propagation is possible in a persistent manner, because lazy
		// propagation works via the combining of updates, which necessarily
		// destroys time information, or otherwise is no longer constant-time.
		//
		// Updates should typically be applied in non-decreasing order of time. One
		// may choose to apply an out-of-order update to operate on a previous
		// "version" of the tree, however, this invalidates later "version"s of the
		// tree. In this method, it is recommended to compute offline the number of
		// versions to be able to revert to.
		//
		// A query for time `t` is evaluated after all requested updates at time `t`
		// have been applied.
		//
		// Additional speedups can be had by offline re-ordering of the updates and
		// applying history pruning in `retrace` and `apply`.
		template <typename Policy, typename TimeType = std::size_t>
		class PolicyPersistentWrapper {
			public:
			using Value = std::map<TimeType, typename Policy::Value>;
			using Update = std::pair<TimeType, typename Policy::Update>;
			using Result = typename Policy::Result;
			using Query = std::pair<TimeType, typename Policy::Query>;

			static inline Value defaultValue() {
				return {{std::numeric_limits<TimeType>::min(), Policy::defaultValue()}};
			}
			static inline Update defaultUpdate() {
				return {std::numeric_limits<TimeType>::min(), Policy::defaultUpdate()};
			}
			static inline Result defaultResult() { return Policy::defaultResult(); }
			static inline Result
			convert(Value const &value, Query const &query, std::size_t size) {
				return Policy::convert(
					std::prev(value.upper_bound(query.first))->second,
					query.second,
					size);
			}
			static inline void combine(Update &current, Update const &update) {
				current.first = update.first;
				Policy::combine(current.second, update.second);
			}
			// TODO: Persistent retrace performs redundant work in binary searching
			// left/right each time, so this is slower than necessary by a
			// constant factor.
			static inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				Update const &update) {
				auto hint{std::prev(value.upper_bound(update.first))};
				auto it{value.insert_or_assign(hint, update.first, hint->second)};
				Policy::retrace(
					it->second,
					std::prev(left.upper_bound(update.first))->second,
					std::prev(right.upper_bound(update.first))->second,
					update.second);
			}
			static inline void
			build(Value &value, Value const &left, Value const &right) {
				auto t{std::max(left.rbegin()->first, right.rbegin()->first)};
				auto hint{std::prev(value.upper_bound(t))};
				auto it{value.insert_or_assign(hint, t, hint->second)};
				Policy::build(
					it->second, left.rbegin()->second, right.rbegin()->second);
			}
			static inline void
			apply(Value &value, Update const &update, std::size_t size) {
				auto hint{std::prev(value.upper_bound(update.first))};
				auto it{value.insert_or_assign(hint, update.first, hint->second)};
				Policy::apply(it->second, update.second, size);
			}
			static inline Result
			aggregate(Result const &left, Result const &right, Query const &query) {
				return Policy::aggregate(left, right, query.second);
			}
		};

		template <typename ValueType>
		class PolicySum : public PolicyBase<ValueType> {
			public:
			using SuperPolicy = PolicyBase<ValueType>;
			using typename SuperPolicy::Value;
			using typename SuperPolicy::Update;
			using typename SuperPolicy::Result;
			using typename SuperPolicy::Query;

			static inline void combine(Update &current, Update const &update) {
				current += update;
			}
			static inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				Update const &) {
				value = left + right;
			}
			static inline void
			build(Value &value, Value const &left, Value const &right) {
				value = left + right;
			}
			static inline void
			apply(Value &value, Update const &update, std::size_t size) {
				value += update * static_cast<Update>(size);
			}
			static inline Result
			aggregate(Result const &left, Result const &right, Query const &) {
				return left + right;
			}
		};

		template <typename ValueType>
		class PolicyMin : public PolicyBase<ValueType> {
			public:
			using SuperPolicy = PolicyBase<ValueType>;
			using typename SuperPolicy::Value;
			using typename SuperPolicy::Update;
			using typename SuperPolicy::Result;
			using typename SuperPolicy::Query;

			static inline Result defaultResult() {
				return std::numeric_limits<Result>::max();
			}
			static inline void combine(Update &current, Update const &update) {
				current += update;
			}
			static inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				Update const &) {
				value = std::min(left, right);
			}
			static inline void
			build(Value &value, Value const &left, Value const &right) {
				value = std::min(left, right);
			}
			static inline void
			apply(Value &value, Update const &update, std::size_t) {
				value += update;
			}
			static inline Result
			aggregate(Result const &left, Result const &right, Query const &) {
				return std::min(left, right);
			}
		};

		template <typename ValueType>
		class PolicyMax : public PolicyBase<ValueType> {
			public:
			using SuperPolicy = PolicyBase<ValueType>;
			using typename SuperPolicy::Value;
			using typename SuperPolicy::Update;
			using typename SuperPolicy::Result;
			using typename SuperPolicy::Query;

			static inline Result defaultResult() {
				return std::numeric_limits<Result>::min();
			}
			static inline void combine(Update &current, Update const &update) {
				current += update;
			}
			static inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				Update const &) {
				value = std::max(left, right);
			}
			static inline void
			build(Value &value, Value const &left, Value const &right) {
				value = std::max(left, right);
			}
			static inline void
			apply(Value &value, Update const &update, std::size_t) {
				value += update;
			}
			static inline Result
			aggregate(Result const &left, Result const &right, Query const &) {
				return std::max(left, right);
			}
		};

		// 2D segtree for point updates and range queries.
		template <typename ValueType, std::size_t INNER_DIMENSION>
		class PolicySum2DPoint : public PolicyBase<
															 FenwickTree<ValueType>,
															 std::pair<std::size_t, ValueType>,
															 ValueType,
															 std::size_t> {
			public:
			using SuperPolicy = PolicyBase<
				FenwickTree<ValueType>,
				std::pair<std::size_t, ValueType>,
				ValueType,
				std::size_t>;
			using typename SuperPolicy::Value;
			using typename SuperPolicy::Update;
			using typename SuperPolicy::Result;
			using typename SuperPolicy::Query;

			static inline Value defaultValue() { return {INNER_DIMENSION}; }
			static inline Result defaultResult() { return {}; }
			static inline Result
			convert(Value const &value, Query const &query, std::size_t) {
				return value.sum(query);
			}
			// combine is omitted because we only support point updates.
			static inline void retrace(
				Value &value,
				Value const &left,
				Value const &right,
				Update const &update) {
				// We can directly apply the update to this vertex.
				value.modify(update.first, update.second);
			}
			// build is omitted because there is no easy way to combine two Fenwicks.
			static inline void
			apply(Value &value, Update const &update, std::size_t) {
				value.modify(update.first, update.second);
			}
			static inline Result
			aggregate(Result const &left, Result const &right, Query const &) {
				return left + right;
			}
		};
	};

	// Segment tree with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// Loosely based on <https://codeforces.com/blog/entry/18051>. Earlier
	// iterations of this data structure have higher constant factor but enable
	// more intuitive modifications. Due to the memory layout, some vertices are
	// "bridge" vertices, and aggregate a prefix and suffix of the underlying
	// together. Bridges do not have a uniform height or size.
	//
	// Inherit and modify the provided default policy to implement custom
	// behavior.
	//
	// Updates must be combinable. If updates are not combinable, consider using
	// the non-lazy version of this segtree, or otherwise using a
	// higher-dimensional structure (e.g. quad-tree). Otherwise, using only point
	// updates will guarantee that combine is never called.
	//
	// build is only required when using the building version of the constructor.
	// retrace receives information about the update that caused it to be called,
	// and may know which child was updated, but this is not yet implemented.
	template <typename Policy>
	class SegmentTreeLazy {
		public:
		using Value = typename Policy::Value;
		using Update = typename Policy::Update;
		using Result = typename Policy::Result;
		using Query = typename Policy::Query;

		using Error = SegmentTreeLazy<>::Error;
		using Exception = SegmentTreeLazy<>::Exception;

		private:
		class Vertex {
			public:
			Value value{Policy::defaultValue()};

			// True iff node has a pending lazy update to propagate to its children.
			// The update has already been applied to the node itself.
			bool lazy{false};

			// Lazily-stored updates. Will be the default update if lazy is false.
			Update update{Policy::defaultUpdate()};
		};

		// Depth of the deepest node in the tree.
		std::size_t const DEPTH;

		// Number of underlying nodes.
		std::size_t const SIZE_UNDERLYING;

		// All vertices in the tree. Vertex 0 is unused.
		mutable std::vector<Vertex> vertices;

		// Propagate a single non-leaf vertex in the tree. It is guaranteed that
		// propagate will never be called on a leaf. propagate may be called on a
		// bridge, but bridges (and ancestors of them) will never be lazy.
		//
		// `size` is only valid for non-bridge-ancestors. It is always the number of
		// leaf nodes contained in this subtree.
		inline void propagate(std::size_t idx, std::size_t size) const {
			if (!this->vertices[idx].lazy) {
				return;
			}

			Policy::apply(
				this->vertices[idx * 2].value, this->vertices[idx].update, size / 2);
			Policy::apply(
				this->vertices[idx * 2 + 1].value,
				this->vertices[idx].update,
				size / 2);

			// Avoid unnecessarily setting lazy on a leaf node.
			if (idx * 2 < this->SIZE_UNDERLYING) {
				Policy::combine(
					this->vertices[idx * 2].update, this->vertices[idx].update);
				Policy::combine(
					this->vertices[idx * 2 + 1].update, this->vertices[idx].update);
				this->vertices[idx * 2].lazy = this->vertices[idx * 2 + 1].lazy = true;
			}

			this->vertices[idx].update = Policy::defaultUpdate();
			this->vertices[idx].lazy = false;
		}

		// Propagate all ancestors of a single vertex, optionally beginning at a
		// specific ancestor depth.
		inline void propagateTo(std::size_t idx) const {
			for (std::size_t level{this->DEPTH}, size{1_zu << level}; level > 0;
					 level--, size /= 2) {
				if ((idx >> level) == 0) {
					continue;
				}
				this->propagate(idx >> level, size);
			}
		}

		public:
		// Publicly accessible way to traverse the tree while abstracting away the
		// lazy propagation. It is assumed that the tree has been propagated to the
		// iterator already.
		//
		// Iterators remain valid as long as no operations are performed between
		// accesses. Operations may invalidate the values of iterators as they will
		// no longer be up-to-date with lazy updates. This can be resolved with
		// `revalidate`.
		//
		// TODO: Implement a constant version of this.
		class Iterator {
			private:
			SegmentTreeLazy<Policy> const *TREE;

			std::size_t IDX, SIZE;

			public:
			// Size is the number of leaf nodes in this subtree. It is only valid if
			// this is not an ancestor of a bridge.
			Iterator(
				SegmentTreeLazy<Policy> const *tree,
				std::size_t idx,
				std::size_t size)
					: TREE{tree}, IDX{idx}, SIZE{size} {}
			Iterator(Iterator const &other)
					: TREE{other.TREE}, IDX{other.IDX}, SIZE{other.SIZE} {}

			Iterator &operator=(Iterator const &other) {
				this->TREE = other.TREE;
				this->IDX = other.IDX;
				this->SIZE = other.SIZE;
				return *this;
			}

			inline bool isRoot() { return this->IDX == 1; }
			inline bool isLeaf() { return this->IDX >= this->TREE->SIZE_UNDERLYING; }
			inline bool isFrontUnderlying() {
				return this->IDX == this->TREE->SIZE_UNDERLYING;
			}
			inline bool isBackUnderlying() {
				return this->IDX == this->TREE->SIZE_UNDERLYING * 2 - 1;
			}

			inline Iterator parent() {
				if (this->isRoot()) {
					throw Exception(Error::ITERATOR_INVALID);
				}
				return {this->TREE, this->IDX / 2, this->SIZE * 2};
			}
			// Children access.
			inline Iterator left() {
				if (this->isLeaf()) {
					throw Exception(Error::ITERATOR_INVALID);
				}
				this->TREE->propagate(this->IDX, this->SIZE);
				return {this->TREE, this->IDX * 2, this->SIZE / 2};
			}
			inline Iterator right() {
				if (this->isLeaf()) {
					throw Exception(Error::ITERATOR_INVALID);
				}
				this->TREE->propagate(this->IDX, this->SIZE);
				return {this->TREE, this->IDX * 2 + 1, this->SIZE / 2};
			}
			// Underlying access. Throws if at the ends of the underlying array, or if
			// this is not leaf. Because of the need to propagate, these are O(\lg N)
			// amortized.
			//
			// TODO: Can this be implemented faster?
			inline Iterator nextUnderlying() {
				if (!this->isLeaf() || this->isBackUnderlying()) {
					throw Exception(Error::ITERATOR_INVALID);
				}
				this->TREE->propagateTo(this->IDX + 1);
				return {this->TREE, this->IDX + 1, this->SIZE};
			}
			inline Iterator prevUnderlying() {
				if (!this->isLeaf() || this->isFrontUnderlying()) {
					throw Exception(Error::ITERATOR_INVALID);
				}
				this->TREE->propagateTo(this->IDX - 1);
				return {this->TREE, this->IDX - 1, this->SIZE};
			}

			// If updates have changed the tree, re-validate this iterator by
			// propagating to it.
			inline void revalidate() { this->TREE->propagateTo(this->IDX); }

			// Access to iterator value.
			Value &operator*() { return this->TREE->vertices[this->IDX].value; }
		};

		friend Iterator;

		// Segment tree for a segment array of size size.
		SegmentTreeLazy(std::size_t size)
				: DEPTH{mostSignificant1BitIdx(size * 2)},
					SIZE_UNDERLYING{size},
					vertices(size * 2) {}

		// Segment tree with all leaf nodes moved in, and the others constructed in
		// order. This minimizes build time by a constant factor.
		SegmentTreeLazy(std::vector<Value> &&values)
				: SegmentTreeLazy(values.size()) {
			for (std::size_t i{0}; i < this->SIZE_UNDERLYING; i++) {
				std::swap(values[i], this->vertices[this->SIZE_UNDERLYING + i].value);
			}
			for (std::size_t i{this->SIZE_UNDERLYING - 1}; i > 0; i--) {
				Policy::build(
					this->vertices[i].value,
					this->vertices[i * 2].value,
					this->vertices[i * 2 + 1].value);
			}
		}

		// Queries an inclusive range, propagating if necessary then aggregating.
		Result query(std::size_t left, std::size_t right, Query const &query = {}) {
			this->propagateTo(left + this->SIZE_UNDERLYING);
			this->propagateTo(right + this->SIZE_UNDERLYING);
			Result resLeft{Policy::defaultResult()},
				resRight{Policy::defaultResult()};
			std::size_t size{1};
			for (left += this->SIZE_UNDERLYING, right += this->SIZE_UNDERLYING + 1;
					 left < right;
					 left /= 2, right /= 2, size *= 2) {
				if (left % 2 == 1) {
					resLeft = Policy::aggregate(
						resLeft,
						Policy::convert(this->vertices[left++].value, query, size),
						query);
				}
				if (right % 2 == 1) {
					resRight = Policy::aggregate(
						Policy::convert(this->vertices[--right].value, query, size),
						resRight,
						query);
				}
			}
			return Policy::aggregate(resLeft, resRight, query);
		}

		// Lazy update an inclusive range. The updated will be applied identically
		// to all nodes in the range, save for differences based on the depth of the
		// node (which will be expressed via the std::size_t range parameter).
		void update(std::size_t left, std::size_t right, Update const &update) {
			this->propagateTo(left + this->SIZE_UNDERLYING);
			this->propagateTo(right + this->SIZE_UNDERLYING);
			// Only retrace updates once left or right node has been changed.
			bool changedLeft{false}, changedRight{false};
			std::size_t size{1};
			for (left += this->SIZE_UNDERLYING, right += this->SIZE_UNDERLYING + 1;
					 left < right;
					 left /= 2, right /= 2, size *= 2) {
				if (changedLeft) {
					Policy::retrace(
						this->vertices[left - 1].value,
						this->vertices[left * 2 - 2].value,
						this->vertices[left * 2 - 1].value,
						update);
				}
				if (changedRight) {
					Policy::retrace(
						this->vertices[right].value,
						this->vertices[right * 2].value,
						this->vertices[right * 2 + 1].value,
						update);
				}
				if (left % 2 == 1) {
					changedLeft = true;
					Policy::apply(this->vertices[left].value, update, size);
					// Avoid unnecessarily setting the lazy update on a leaf node.
					if (left < this->SIZE_UNDERLYING) {
						Policy::combine(this->vertices[left].update, update);
						this->vertices[left].lazy = true;
					}
					left++;
				}
				if (right % 2 == 1) {
					right--;
					changedRight = true;
					Policy::apply(this->vertices[right].value, update, size);
					if (right < this->SIZE_UNDERLYING) {
						Policy::combine(this->vertices[right].update, update);
						this->vertices[right].lazy = true;
					}
				}
			}
			for (left--; right > 0; left /= 2, right /= 2) {
				if (changedLeft && left > 0) {
					Policy::retrace(
						this->vertices[left].value,
						this->vertices[left * 2].value,
						this->vertices[left * 2 + 1].value,
						update);
				}
				if (changedRight && (!changedLeft || left != right)) {
					Policy::retrace(
						this->vertices[right].value,
						this->vertices[right * 2].value,
						this->vertices[right * 2 + 1].value,
						update);
				}
			}
		}

		Iterator root() {
			this->propagateTo(1);
			return Iterator(this, 1, 1_zu << this->DEPTH);
		}
		Iterator frontUnderlying() {
			this->propagateTo(this->SIZE_UNDERLYING);
			return Iterator(this, this->SIZE_UNDERLYING, 1);
		}
		Iterator backUnderlying() {
			this->propagateTo(this->SIZE_UNDERLYING * 2 - 1);
			return Iterator(this, this->SIZE_UNDERLYING * 2 - 1, 1);
		}
	};

	template <typename Policy>
	using SegmentTreeLazyPersistent =
		SegmentTreeLazy<SegmentTreeLazy<>::PolicyPersistentWrapper<Policy>>;
}

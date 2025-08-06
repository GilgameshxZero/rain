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
		//
		// This default policy represents a sum tree.
		template <
			typename ValueType,
			typename UpdateType = ValueType,
			typename ResultType = ValueType,
			typename QueryType = std::nullptr_t>
		class Policy : public PolicyBaseDefaultValueResult<ValueType, ResultType>,
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
		};
	};

	// Segment tree with lazy propagation, supporting range queries and range
	// updates in O(ln N) and O(N) memory.
	//
	// Loosely based on <https://codeforces.com/blog/entry/18051>. Earlier
	// iterations of this data structure have higher constant factor but enable
	// more intuitive modifications. Inherit and modify the provided default
	// policy to implement custom behavior.
	//
	// If only point queries are used, Policy::combine can just overwrite the
	// existing update and the tree will not be lazy.
	template <typename Policy>
	class SegmentTreeLazy {
		public:
		using Value = typename Policy::Value;
		using Update = typename Policy::Update;
		using Result = typename Policy::Result;
		using Query = typename Policy::Query;

		private:
		class Vertex {
			public:
			mutable Value value{Policy::defaultValue()};

			// True iff node has a pending lazy update to propagate to its children.
			// The update has already been applied to the node itself.
			mutable bool lazy{false};

			// Lazily-stored updates. Will be the default update if lazy is false.
			mutable Update update{Policy::defaultUpdate()};

			// Underlying inclusive range of the vertex. May be invalid if the range
			// bridges both ends of the range.
			std::pair<std::size_t, std::size_t> range{
				std::numeric_limits<std::size_t>::max(),
				std::numeric_limits<std::size_t>::max()};

			// Number of leaf vertices in this vertex's subtree.
			std::size_t size{1};
		};

		// Height of the highest node in the tree.
		std::size_t const HEIGHT;

		// All vertices in the tree. Vertex 0 is unused.
		std::vector<Vertex> vertices;

		// Propagate a single non-leaf vertex in the tree.
		void propagate(std::size_t i) const {
			if (!this->vertices[i].lazy) {
				return;
			}

			Policy::apply(
				this->vertices[i * 2].value,
				this->vertices[i].update,
				this->vertices[i * 2].size);
			Policy::apply(
				this->vertices[i * 2 + 1].value,
				this->vertices[i].update,
				this->vertices[i * 2 + 1].size);
			Policy::combine(this->vertices[i * 2].update, this->vertices[i].update);
			Policy::combine(
				this->vertices[i * 2 + 1].update, this->vertices[i].update);
			this->vertices[i * 2].lazy = this->vertices[i * 2 + 1].lazy = true;

			this->vertices[i].update = Policy::defaultUpdate();
			this->vertices[i].lazy = false;
		}

		// Propagate all ancestors of a single vertex.
		// After this, no ancestor of any node in this range should have a queued
		// update.
		void propagateTo(std::size_t idx) const {
			for (std::size_t level{this->HEIGHT}; level > 0; --level) {
				this->propagate((idx + this->vertices.size() / 2) >> level);
			}
		}

		public:
		// Segment tree for a segment array of size size.
		SegmentTreeLazy(std::size_t const size)
				: HEIGHT{mostSignificant1BitIdx(2 * size)}, vertices(2 * size) {
			// Set size and range.
			this->vertices.shrink_to_fit();
			for (std::size_t i{0}; i < size; i++) {
				this->vertices[size + i].range = {i, i};
			}
			for (std::size_t i{size - 1}; i > 0; i--) {
				this->vertices[i].size =
					this->vertices[i * 2].size + this->vertices[i * 2 + 1].size;
				if (
					this->vertices[i * 2].range.second + 1 ==
					this->vertices[i * 2 + 1].range.first) {
					this->vertices[i].range = {
						this->vertices[i * 2].range.first,
						this->vertices[i * 2 + 1].range.second};
				}
			}
		}

		// Segment tree with all leaf nodes moved in, and the others constructed in
		// order. This minimizes build time by a constant factor.
		SegmentTreeLazy(std::vector<Value> &&values)
				: SegmentTreeLazy(values.size()) {
			for (std::size_t i{0}; i < values.size(); i++) {
				std::swap(values[i], this->vertices[values.size() + i].value);
			}
			for (std::size_t i{values.size() - 1}; i > 0; i--) {
				Policy::retrace(
					this->vertices[i].value,
					this->vertices[i * 2].value,
					this->vertices[i * 2 + 1].value);
			}
		}

		// Queries an inclusive range, propagating if necessary then aggregating.
		// Take an optional query parameter `query` which will be used in convert
		// and aggregate of the results and values.
		Result query(std::size_t left, std::size_t right, Query const &query = {})
			const {
			this->propagateTo(left);
			this->propagateTo(right);
			Result resLeft{Policy::defaultResult()},
				resRight{Policy::defaultResult()};
			for (left += this->vertices.size() / 2,
					 right += this->vertices.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2) {
				if (left % 2 == 1) {
					resLeft = Policy::aggregate(
						resLeft,
						Policy::convert(
							this->vertices[left].value, query, this->vertices[left].size),
						query);
					left++;
				}
				if (right % 2 == 1) {
					right--;
					resRight = Policy::aggregate(
						Policy::convert(
							this->vertices[right].value, query, this->vertices[right].size),
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
			// We must propagate here because retrace expects non-lazy nodes to store
			// the value.
			this->propagateTo(left);
			this->propagateTo(right);
			// Only retrace updates once left or right node has been changed.
			bool changedLeft{false}, changedRight{false};
			for (left += this->vertices.size() / 2,
					 right += this->vertices.size() / 2 + 1;
					 left < right;
					 left /= 2, right /= 2) {
				if (changedLeft) {
					Policy::retrace(
						this->vertices[left - 1].value,
						this->vertices[left * 2 - 2].value,
						this->vertices[left * 2 - 1].value);
				}
				if (changedRight) {
					Policy::retrace(
						this->vertices[right].value,
						this->vertices[right * 2].value,
						this->vertices[right * 2 + 1].value);
				}
				if (left % 2 == 1) {
					Policy::apply(
						this->vertices[left].value, update, this->vertices[left].size);
					Policy::combine(this->vertices[left].update, update);
					this->vertices[left].lazy = true;
					changedLeft = true;
					left++;
				}
				if (right % 2 == 1) {
					right--;
					Policy::apply(
						this->vertices[right].value, update, this->vertices[right].size);
					Policy::combine(this->vertices[right].update, update);
					this->vertices[right].lazy = true;
					changedRight = true;
				}
			}
			for (left--; right > 0; left /= 2, right /= 2) {
				if (changedLeft) {
					Policy::retrace(
						this->vertices[left].value,
						this->vertices[left * 2].value,
						this->vertices[left * 2 + 1].value);
				}
				if (changedRight && (!changedLeft || left != right)) {
					Policy::retrace(
						this->vertices[right].value,
						this->vertices[right * 2].value,
						this->vertices[right * 2 + 1].value);
				}
			}
		}

		// Get the state of a single vertex, useful for traversing the tree.
		// TODO: implement sane iterators.
		Vertex const &getVertex(std::size_t idx) const {
			return this->vertices[idx];
		}
	};

	template <typename ValueType>
	class SegmentTreeLazySumPolicy : public SegmentTreeLazy<>::Policy<ValueType> {
		public:
		using typename SegmentTreeLazy<>::Policy<ValueType>::Value;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Update;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Result;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Query;

		static inline void combine(Update &current, Update const &update) {
			current += update;
		}
		static inline void
		retrace(Value &value, Value const &left, Value const &right) {
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
	using SegmentTreeLazySum =
		SegmentTreeLazy<SegmentTreeLazySumPolicy<ValueType>>;

	template <typename ValueType>
	class SegmentTreeLazyMinPolicy : public SegmentTreeLazy<>::Policy<ValueType> {
		public:
		using typename SegmentTreeLazy<>::Policy<ValueType>::Value;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Update;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Result;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Query;

		static inline Result defaultResult() {
			return std::numeric_limits<Result>::max();
		}
		static inline void combine(Update &current, Update const &update) {
			current += update;
		}
		static inline void
		retrace(Value &value, Value const &left, Value const &right) {
			value = std::min(left, right);
		}
		static inline void apply(Value &value, Update const &update, std::size_t) {
			value += update;
		}
		static inline Result
		aggregate(Result const &left, Result const &right, Query const &) {
			return std::min(left, right);
		}
	};

	template <typename ValueType>
	using SegmentTreeLazyMin =
		SegmentTreeLazy<SegmentTreeLazyMinPolicy<ValueType>>;

	template <typename ValueType>
	class SegmentTreeLazyMaxPolicy : public SegmentTreeLazy<>::Policy<ValueType> {
		public:
		using typename SegmentTreeLazy<>::Policy<ValueType>::Value;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Update;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Result;
		using typename SegmentTreeLazy<>::Policy<ValueType>::Query;

		static inline Result defaultResult() {
			return std::numeric_limits<Result>::min();
		}
		static inline void combine(Update &current, Update const &update) {
			current += update;
		}
		static inline void
		retrace(Value &value, Value const &left, Value const &right) {
			value = std::max(left, right);
		}
		static inline void apply(Value &value, Update const &update, std::size_t) {
			value += update;
		}
		static inline Result
		aggregate(Result const &left, Result const &right, Query const &) {
			return std::max(left, right);
		}
	};

	template <typename ValueType>
	using SegmentTreeLazyMax =
		SegmentTreeLazy<SegmentTreeLazyMaxPolicy<ValueType>>;
}

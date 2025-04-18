// Least-recently-used cache implemented with a linked list + hashmap. O(1)
// average access. Not thread-safe.
#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace Rain::Algorithm {
	// Least-recently-used cache implemented with a linked list + hashmap. O(1)
	// average access. Not thread-safe.
	//
	// Value will be moved constructed if passed as rvalue-reference. Same with
	// key. Otherwise, they will be copy-constructed. Ensure the relevant
	// constructors are defined.
	template <
		typename Key,
		typename Value,
		typename Hash = std::hash<Key>,
		typename KeyEqual = std::equal_to<Key>>
	class LruCache {
		private:
		typedef std::list<std::pair<Key const &, Value>> InternalList;
		typedef std::
			unordered_map<Key, typename InternalList::iterator, Hash, KeyEqual>
				InternalHashMap;

		// List stores key/value pairs in LRU order; hashMap looks up list iterators
		// based on key. Key reference in list points to key owned by hashMap.
		InternalList lruList;
		InternalHashMap hashMap;

		public:
		// Key/value pair capacity of the cache. Cannot be 0.
		std::size_t const capacity;

		LruCache(std::size_t const capacity) : capacity(capacity) {}

		// Simple getters.
		std::size_t size() const noexcept { return this->lruList.size(); }
		bool empty() const noexcept { return this->lruList.empty(); }
		auto begin() const noexcept { return this->lruList.begin(); }
		auto end() const noexcept { return this->lruList.end(); }
		auto begin() noexcept { return this->lruList.begin(); }
		auto end() noexcept { return this->lruList.end(); }

		// Returns an iterator to the hashmap object if available, otherwise
		// this->end(). Updates LRU ordering.
		typename InternalList::iterator find(Key const &key) {
			typename InternalHashMap::iterator const findIt{this->hashMap.find(key)};

			if (findIt == this->hashMap.end()) {
				return this->end();
			}

			this->lruList.splice(
				this->lruList.begin(), this->lruList, findIt->second);
			return this->lruList.begin();
		}

		// Access the value at a key. Throws if doesn't exist.
		Value &at(Key const &key) {
			auto findIt = this->find(key);
			if (findIt == this->end()) {
				throw std::out_of_range("Key does not exist in cache.");
			}
			return findIt->second;
		}

		// Insert a new key/value pair, or update an existing pair. In either case,
		// move the pair to the front of the cache.
		//
		// Arguments are forwarded to relevant constructors with universal
		// forwarding. std::move them in to move construct; otherwise, they will be
		// copy-constructed.
		template <typename KeyType, typename ValueType>
		std::pair<typename InternalList::iterator, bool> insertOrAssign(
			KeyType &&key,
			ValueType &&value) {
			typename InternalHashMap::iterator const findIt{this->hashMap.find(key)};

			if (findIt != this->hashMap.end()) {
				// If key exists, simply move it to the front of the list.
				this->lruList.splice(
					this->lruList.begin(), this->lruList, findIt->second);

				// Move-assign value in; key does not need to be changed.
				this->lruList.begin()->second = std::move(value);
				return {this->lruList.begin(), false};
			}

			// If over capacity, evict.
			if (this->hashMap.size() >= this->capacity) {
				this->hashMap.erase(this->lruList.back().first);
				this->lruList.pop_back();
			}

			// Insert into hashMap first, with default value iterator.
			typename InternalHashMap::iterator mapIt;
			std::tie(mapIt, std::ignore) =
				this->hashMap.emplace(std::forward<KeyType>(key), this->lruList.end());

			// Insert value with key reference.
			this->lruList.emplace_front(mapIt->first, std::forward<ValueType>(value));

			// Give hashMap the correct iterator.
			mapIt->second = this->lruList.begin();
			return {this->lruList.begin(), true};
		}
	};
}

// Thread-safe least-recently-used cache implemented with a linked list +
// hashmap. O(1) average access.
#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace Rain::Algorithm {
	// Thread-safe LRU cache. Keys should be light and copy-constructable.
	template <typename Key, typename Value>
	class LruCache : public std::unordered_map<
										 Key,
										 typename std::list<
											 std::unique_ptr<std::pair<Key, Value>>>::iterator> {
		private:
		// Shorthands.
		typedef std::list<std::unique_ptr<std::pair<Key, Value>>> List;
		typedef std::unordered_map<Key, typename List::iterator> Super;

		// Linked list storing key/value pairs, in LRU order.
		List lruList;

		// Mutex locks operations on the linked list.
		std::mutex lruListMtx;

		public:
		// Key/value pair capacity of the cache. Zero = unlimited, which is a little
		// meaningless.
		std::size_t const capacity;

		LruCache(std::size_t capacity = 0) : capacity(capacity) {}

		// Overloaded for member access related functions on the base
		// std::unordered_map.
		Value &at(Key const &key) {
			std::lock_guard<std::mutex> lck(this->lruListMtx);

			// Throws exception from std::unordered_map if key is not in cache.
			typename List::iterator &it = Super::at(key);

			// Move this key/value pair to the beginning of the linked list. Update
			// the unordered_map to match.
			this->lruList.splice(this->lruList.begin(), this->lruList, it);

			return this->lruList.front()->second;
		}

		// Insert a new key/value pair, or update an existing pair. In either case,
		// move the pair to the front of the cache.
		std::pair<typename Super::iterator, bool> insert_or_assign(
			Key const &key,
			Value const &value) {
			std::lock_guard<std::mutex> lck(this->lruListMtx);

			typename Super::iterator const &it = Super::find(key);

			// If key exists, erase it to prepare for update later.
			if (it != Super::end()) {
				this->lruList.erase(it->second);
				Super::erase(it);
			}

			// Evict from LRU if over capacity.
			if (Super::size() == this->capacity && this->capacity != 0) {
				Super::erase(this->lruList.back()->first);
				this->lruList.pop_back();
			}

			// Insert new key with value.
			this->lruList.push_front(
				std::make_unique<std::pair<Key, Value>>(key, value));
			return Super::insert(std::make_pair(key, this->lruList.begin()));
		}

		// Destructor needs to free all dynamically-allocated memory in the linked
		// list.
		~LruCache() {
			while (this->lruList.size() > 0) {
				this->lruList.pop_back();
			}
		}
	};
}

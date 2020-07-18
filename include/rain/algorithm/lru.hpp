// A base class with immutable LRU cache behavior, to be subclassed by similar
// caches.
#pragma once

#include "../types.hpp"

#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace Rain::Algorithm {
	// KeyType should be light. ValueType can be heavier, but the cache is
	// immutable w.r.t. ValueType.
	template <typename KeyType, typename ValueType>
	class LRUCache
			: private std::unordered_map<KeyType,
					typename std::list<std::pair<KeyType, ValueType>>::iterator> {
		private:
		// Shorthand.
		typedef std::list<std::pair<KeyType, ValueType>> ListType;
		typedef std::unordered_map<KeyType, typename ListType::iterator> Super;

		public:
		const std::size_t capacity;

		// Zero capacity means infinite.
		LRUCache(std::size_t capacity = 0) : capacity(capacity) {}
		ValueType &at(const KeyType &key) {
			std::lock_guard<std::mutex> lck(this->mtx);
			const typename Super::iterator &it = Super::find(key);
			if (it == Super::end()) {
				throw std::out_of_range("Key does not exist in LRU cache.");
			}
			this->lru.push_front(*(it->second));
			this->lru.erase(it->second);
			it->second = this->lru.begin();
			return this->lru.begin()->second;
		}
		std::pair<typename Super::iterator, bool> insert_or_assign(
			const KeyType &key,
			const ValueType &value) {
			std::lock_guard<std::mutex> lck(this->mtx);

			const typename Super::iterator &it = Super::find(key);
			if (it != Super::end()) {	 // If key already exists, replace it.
				this->lru.erase(it->second);
				Super::erase(it);
			}
			if (Super::size() == this->capacity) {	// Evict LRU if over capacity.
				Super::erase(this->lru.back().first);
				this->lru.pop_back();
			}

			// Insert new key.
			this->lru.push_front(std::make_pair(key, value));
			return Super::insert(std::make_pair(key, this->lru.begin()));
		}

		private:
		ListType lru;
		std::mutex mtx;
	};
}

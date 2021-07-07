/*
Thread-safe base class with immutable LRU cache behavior. Can be subclassed.
*/

#pragma once

#include "../error-exception/exception.hpp"
#include "../type.hpp"

#include <list>
#include <mutex>
#include <unordered_map>

namespace Rain::Algorithm {
	/*
	KeyType should be light. ValueType can be heavier, but the cache is immutable
	w.r.t. ValueType.
	*/
	template <typename KeyType, typename ValueType>
	class LruCache
			: private std::unordered_map<KeyType,
					typename std::list<std::pair<KeyType, ValueType>>::iterator> {
		private:
		// Shorthand typedefs.
		typedef std::list<std::pair<KeyType, ValueType>> ListType;
		typedef std::unordered_map<KeyType, typename ListType::iterator> Super;

		public:
		std::size_t const capacity;

		// Zero capacity means infinite, which is a little meaningless.
		LruCache(std::size_t capacity = 0) : capacity(capacity) {}

		// Overload for member access.
		ValueType &at(KeyType const &key) {
			std::lock_guard<std::mutex> lck(this->mtx);
			
			// Throws exception if key is not in cache.
			typename ListType::iterator &it = Super::at(key);

			this->lru.push_front(*it);
			this->lru.erase(it);
			it = this->lru.begin();
			return this->lru.begin()->second;
		}

		// Custom function for LruCache.
		std::pair<typename Super::iterator, bool> insert_or_assign(
			KeyType const &key,
			ValueType const &value) {
			std::lock_guard<std::mutex> lck(this->mtx);
			typename Super::iterator const &it = Super::find(key);
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

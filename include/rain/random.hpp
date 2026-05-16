#pragma once

#include "functional/trait.hpp"

#include <chrono>
#include <random>

namespace Rain::Random {
	template<typename, typename = void>
	struct SplitMixHash;

	// Simple function to combine two 32 or 64-bit hashes,
	// based on
	// <https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes>
	// from Boost.
	//
	// SIZE_T_SIZE is a default argument which forces a
	// substitution, and thus SFINAE.
	template<
		std::size_t SIZE_T_SIZE = sizeof(std::size_t),
		typename std::enable_if<SIZE_T_SIZE == 8>::type * =
			nullptr>
	static inline std::size_t combineHash(
		std::size_t &seed,
		std::size_t next) {
		return seed ^=
			next + 0x517cc1b727220a95 + (seed << 6) + (seed >> 2);
	}
	template<
		typename Type,
		std::size_t SIZE_T_SIZE = sizeof(std::size_t),
		typename std::enable_if<SIZE_T_SIZE == 4>::type * =
			nullptr>
	static inline std::size_t combineHash(
		std::size_t &seed,
		std::size_t next) {
		return seed ^=
			next + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	// SFINAE enables SplitMixHash for all std::hash-able
	// types, and defines custom unwrapping hash for
	// pairs/containers/etc.
	template<typename Type>
	struct SplitMixHash<
		Type,
		typename std::enable_if<
			Functional::TypeTrait<Type>::IsStdHashable::value &&
			sizeof(std::size_t) == 8>::type> {
		// 64-bit hash from
		// <https://codeforces.com/blog/entry/62393>.
		std::size_t operator()(Type const &value) const {
			static std::mt19937_64 generator(
				std::random_device{}());
			static const std::size_t FIXED_RANDOM(generator());
			std::size_t hash{
				std::hash<typename std::decay<Type>::type>{}(
					value)};
			hash += FIXED_RANDOM + 0x9e3779b97f4a7c15;
			hash = (hash ^ (hash >> 30)) * 0xbf58476d1ce4e5b9;
			hash = (hash ^ (hash >> 27)) * 0x94d049bb133111eb;
			return hash ^ (hash >> 31);
		}
	};

	template<typename Type>
	struct SplitMixHash<
		Type,
		typename std::enable_if<
			Functional::TypeTrait<Type>::IsStdHashable::value &&
			sizeof(std::size_t) == 4>::type> {
		// 32-bit hash from
		// <https://groups.google.com/g/prng/c/VFjdFmbMgZI>.
		std::size_t operator()(Type const &value) const {
			static std::mt19937 generator(std::random_device{}());
			static const std::size_t FIXED_RANDOM(generator());
			std::size_t hash{
				std::hash<typename std::decay<Type>::type>{}(
					value)};
			hash += FIXED_RANDOM + 0x9e3779b9;
			hash = (hash ^ (hash >> 16)) * 0x85ebca6b;
			hash = (hash ^ (hash >> 13)) * 0xc2b2ae35;
			return hash ^ (hash >> 16);
		}
	};
	template<typename Type>
	struct SplitMixHash<
		Type,
		typename std::enable_if<
			!Functional::TypeTrait<Type>::IsStdHashable::value &&
			Functional::TypeTrait<Type>::IsConstIterable::value>::
			type> {
		// Unwraps a container.
		std::size_t operator()(Type const &value) const {
			std::size_t result{};
			for (auto const &i : value) {
				combineHash(result, SplitMixHash<decltype(i)>{}(i));
			}
			return result;
		}
	};
	template<typename Type>
	struct SplitMixHash<
		Type,
		typename std::enable_if<
			!Functional::TypeTrait<Type>::IsStdHashable::value &&
			!Functional::TypeTrait<
				Type>::IsConstIterable::value>::type> {
		// Unwraps a pair.
		std::size_t operator()(Type const &value) const {
			// MSVC bug: putting this valid SFINAE expression
			// within the class or function template just nukes
			// the entire class specialization. Instead, putting
			// it in the body seems to work.
			//
			// TODO: file it.
			if constexpr (
				Functional::TypeTrait<Functional::TypeDowngrade<
					std::pair>>::IsTemplateBaseOf<Type>::value) {
				std::size_t result{};
				combineHash(
					result,
					SplitMixHash<decltype(value.first)>{}(
						value.first));
				combineHash(
					result,
					SplitMixHash<decltype(value.second)>{}(
						value.second));
				return result;
			} else {
				// Non-pair, non-const-iterable, non-std-hashable
				// SplitMixHash is not yet implemented!
				static_assert(false);
				return {};
			}
		}
	};
}

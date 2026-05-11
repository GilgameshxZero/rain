// Serializes and deserializes data to/from std::iostream.
#pragma once

#include "../algorithm/bit_manipulators.hpp"
#include "../literal.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace Rain::Data {
	template<typename>
	Functional::TraitFalse isSerializable(...);
	template<typename DataType>
	Functional::TraitTrue isSerializable(
		typename DataType::Serializer *);
	template<typename DataType>
	using IsSerializable =
		decltype(isSerializable<DataType>(nullptr));

	// Serializes and deserializes data to/from std::iostream.
	//
	// Custom types are (de)serialized with free functions
	// (see later).
	//
	// Memory addresses are not remapped during
	// (de)serialization, and thus if your structure contains
	// memory addresses, it must be handled specially.
	class Serializer : public std::ostream {
		private:
		// Manages a subclass of iostream in case we need their
		// functionality.
		std::unique_ptr<std::ostream> streambufManager;

		public:
		Serializer(std::streambuf *streambuf) :
			std::ostream(streambuf) {}
		Serializer(std::string const &filename) :
			std::ostream(nullptr),
			streambufManager(
				new std::ofstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Operators redirect to a nested type, and provides a
		// default.
		template<
			typename DataType,
			std::enable_if<IsSerializable<DataType>::VALUE>::type
				* = nullptr>
		inline Serializer &operator<<(DataType const &data) {
			return DataType::Serializer::serialize(*this, &data);
		}
		template<
			typename DataType,
			std::enable_if<!IsSerializable<DataType>::VALUE>::type
				* = nullptr>
		inline Serializer &operator<<(DataType const &data) {
			Algorithm::writeBytes(
				*this, data, std::endian::little);
			return *this;
		}
		template<
			typename CharT,
			typename Traits,
			typename Allocator>
		inline auto &operator<<(
			std::basic_string<CharT, Traits, Allocator> const
				&data) {
			*this << data.size();
			this->write(
				reinterpret_cast<char const *>(data.data()),
				sizeof(CharT) * data.size());
			return *this;
		}
		template<typename T>
		inline auto &operator<<(
			std::shared_ptr<T> const &data) {
			return *this << *data.get();
		}
		template<typename T, typename Allocator>
		inline auto &operator<<(
			std::vector<T, Allocator> const &data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
		template<
			typename Key,
			typename Compare,
			typename Allocator>
		inline auto &operator<<(
			std::set<Key, Compare, Allocator> const &data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
		template<
			typename Key,
			typename Hash,
			typename KeyEqual,
			typename Allocator>
		inline auto &operator<<(std::
				unordered_set<Key, Hash, KeyEqual, Allocator> const
					&data) {
			*this << data.size();
			for (auto &i : data) {
				*this << i;
			}
			return *this;
		}
	};

	class Deserializer : public std::istream {
		private:
		// Manages a subclass of iostream in case we need their
		// functionality.
		std::unique_ptr<std::istream> streambufManager;

		public:
		Deserializer(std::streambuf *streambuf) :
			std::istream(streambuf) {}
		Deserializer(std::string const &filename) :
			std::istream(nullptr),
			streambufManager(
				new std::ifstream(filename, std::ios::binary)) {
			this->rdbuf(this->streambufManager->rdbuf());
		}

		// Operators redirect to a template type which can be
		// overloaded, and provides a default.
		template<
			typename DataType,
			std::enable_if<IsSerializable<DataType>::VALUE>::type
				* = nullptr>
		inline Deserializer &operator>>(DataType &data) {
			return DataType::Serializer::deserialize(
				*this, &data);
		}
		template<
			typename DataType,
			std::enable_if<!IsSerializable<DataType>::VALUE>::type
				* = nullptr>
		inline Deserializer &operator>>(DataType &data) {
			Algorithm::readBytes(
				*this, data, std::endian::little);
			return *this;
		}
		template<
			typename CharT,
			typename Traits,
			typename Allocator>
		inline auto &operator>>(
			std::basic_string<CharT, Traits, Allocator> &data) {
			std::size_t size;
			*this >> size;
			data.resize(size);
			this->read(
				reinterpret_cast<char *>(&data[0]),
				sizeof(CharT) * size);
			return *this;
		}
		// shared_ptr must already be allocated. We do not
		// allocate since sometimes T is abstract.
		template<typename T>
		inline auto &operator>>(std::shared_ptr<T> &data) {
			return *this >> *data.get();
		}
		template<typename T, typename Allocator>
		inline auto &operator>>(
			std::vector<T, Allocator> &data) {
			std::size_t size;
			*this >> size;
			data.resize(size);
			for (auto &i : data) {
				*this >> i;
			}
			return *this;
		}
		template<
			typename Key,
			typename Compare,
			typename Allocator>
		inline auto &operator>>(
			std::set<Key, Compare, Allocator> &data) {
			std::size_t size;
			*this >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				*this >> key;
				data.insert(key);
			}
			return *this;
		}
		template<
			typename Key,
			typename Hash,
			typename KeyEqual,
			typename Allocator>
		inline auto &operator>>(
			std::unordered_set<Key, Hash, KeyEqual, Allocator>
				&data) {
			std::size_t size;
			*this >> size;
			Key key;
			for (std::size_t i{0}; i < size; i++) {
				*this >> key;
				data.insert(key);
			}
			return *this;
		}
	};
}
